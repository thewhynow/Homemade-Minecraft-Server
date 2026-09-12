# Player / World / Chunk design notes

Notes from a design discussion (2026-09-12) about getting from the play state
to an actual logged-in player.

## Decisions

- `player` is independent of `connection`. The connection is transport
  (socket, framing, handshake/login/configuration state machine); the world
  owns game state.
- Not every connection has a player (status pings, clients still in
  login/configuration), so the link is nullable in both directions.
- Single dimension only, so there is no separate `level` class: `world` holds
  everything.

## Ownership

```
server ── owns ──> connections        (sockets, framing, state machine)
   │
   └──── owns ──> world ── owns ──> players / entities / chunks

connection ··> player*     (null until the client reaches play)
player     ··> connection* (null once the client dropped)
```

- Create the player when the client acknowledges `finish_configuration`
  (the `state = play` transition in `connection::handle_configuration`). The
  world allocates the entity ID used in Login (play).
- `connection::handle_play` decodes packets and calls into player/world; no
  game logic in `connection`.

### Watch out for

1. **Pointer stability.** `std::vector<connection>` by value moves objects on
   `emplace_back`/`erase`, so a `player`'s `connection*` would dangle. Use
   `std::vector<std::unique_ptr<connection>>` (or IDs + lookup).
2. **One teardown path.** When a connection dies, `remove_connections` calls
   `world.remove_player(...)` *before* destroying the connection; that unlinks
   both pointers and tells other players the entity is gone. A kick goes the
   other way: world marks the connection dead, same cleanup path.
3. **Real ticks.** `poll(..., 50)` returns early on any I/O, so it isn't
   20 TPS. Keep a `steady_clock` deadline, call `world.tick()` when it
   passes, and use the remaining time as the poll timeout.
4. `queue_packet` is private on `connection`; world/player need a public send.

## `world`

```cpp
class world {
public:
    static world instance; /* matches server::instance */

    void tick();

    /* players */
    player &add_player(connection &conn, uuid id, std::string name);
    void remove_player(player &p);
    void on_player_moved(player &p);

    /* blocks & chunks */
    chunk &get_or_load_chunk(chunk_pos pos);
    uint16_t get_block(int32_t x, int32_t y, int32_t z);
    void set_block(int32_t x, int32_t y, int32_t z, uint16_t state);

    template<typename T>
    void broadcast(const T &packet);

    int32_t next_entity_id() { return entity_counter++; }
private:
    world();
private:
    static constexpr int32_t min_y  = -64;
    static constexpr int32_t height = 384;

    int64_t hashed_seed;
    int64_t game_time = 0;
    int64_t day_time  = 0;

    int32_t spawn_x = 0, spawn_y = 64, spawn_z = 0;
    float   spawn_angle = 0;

    int32_t entity_counter = 1;
    std::unordered_map<int32_t, std::unique_ptr<player>> players;
    std::unordered_map<chunk_pos, std::unique_ptr<chunk>> chunks;
};
```

Responsibilities:

- **Identity:** dimension key/type (constants, but still sent in Login (play)),
  hashed seed, `min_y`/`height`.
- **Time:** `game_time`, `day_time`, sent via Update Time.
- **Chunks:** `get_or_load_chunk` backed by a generator (flat/void first, disk
  later behind the same function); unload chunks no player can see.
- **Entities:** owns players (later non-player entities too, or one map of
  `unique_ptr<entity>` once `player` inherits `entity`).
- **Spawn:** position and angle.
- **`tick()`:** advance time, tick entities, send movement updates.
- **`on_player_moved`:** on chunk-boundary crossing, send Set Center Chunk,
  send chunks that entered range, unload ones that left.

Stays out of `world`: sockets and packet parsing, registries (already
`synced_registries`), and each client's set of loaded chunks (on `player`).

### Join sequence (verify against protocol docs for your version)

1. Login (play)
2. Game Event "start waiting for level chunks"
3. Set Center Chunk
4. Chunk Data and Update Light for each chunk in view distance
5. Synchronize Player Position (client replies with teleport confirm)

## `player`

```cpp
class player {
public:
    player(connection *conn, int32_t entity_id, uuid id, std::string name);

    void send(const auto &packet);  /* no-op if conn is null */
    chunk_pos current_chunk() const;
    void teleport(double x, double y, double z, float yaw, float pitch);

public:
    /* identity */
    int32_t     entity_id;
    uuid        id;
    std::string name;

    /* link back to the client; null once disconnected */
    connection *conn;

    /* position */
    double x, y, z;
    float  yaw, pitch;
    bool   on_ground;

    /* last position broadcast to other players, to compute movement deltas */
    double sent_x, sent_y, sent_z;

    /* from client_information */
    int32_t view_distance;

    /* what the client has */
    std::unordered_set<chunk_pos> loaded_chunks;
    chunk_pos last_center;

    /* synchronize position handshake */
    int32_t pending_teleport_id = -1;

    /* game state */
    uint8_t gamemode;
    /* health, food, inventory, etc. later */
};
```

- **`view_distance`:** `client_information` is currently received and
  discarded; store it on the connection and pass it in when creating the
  player.
- **`loaded_chunks` / `last_center`:** let `on_player_moved` diff old vs new
  view area.
- **`pending_teleport_id`:** after Synchronize Player Position, ignore client
  movement until it confirms that ID, or its stale position overwrites yours.
- **`sent_x/y/z`:** movement packets are deltas, so track what others were
  last told.
- **Keep-alive** belongs on `connection` (network concern, also applies during
  configuration).
- Later: move entity ID, position, rotation, `on_ground`, `sent_*` into an
  `entity` base class.

## `chunk`

A 16×16 column split into 16×16×16 sections. Simple in-memory format; protocol
encoding only in `serialize`.

```cpp
struct chunk_pos {
    int32_t x, z;
    bool operator==(const chunk_pos &) const = default;
};
/* + std::hash<chunk_pos> specialization */

struct chunk_section {
    std::array<uint16_t, 4096> blocks{}; /* index: (y << 8) | (z << 4) | x */
    std::array<uint8_t, 64>    biomes{}; /* 4x4x4 */
    int16_t non_air_count = 0;           /* the packet needs this per section */

    uint16_t get(int x, int y, int z) const;
    void     set(int x, int y, int z, uint16_t state); /* keeps non_air_count updated */
};

class chunk {
public:
    chunk(chunk_pos pos, int32_t min_y, int32_t height);

    /* local coords: x,z in 0..15, y is absolute */
    uint16_t get_block(int x, int y, int z) const;
    void     set_block(int x, int y, int z, uint16_t state);

    void serialize(std::vector<uint8_t> &out) const; /* body of Chunk Data */

public:
    chunk_pos pos;
    std::vector<chunk_section> sections; /* height / 16, bottom to top */

    /* heightmaps: highest non-air block per column, 16x16 */
    std::array<int16_t, 256> world_surface;
    std::array<int16_t, 256> motion_blocking;

    /* light, per section; can just be full sky light at first */
    /* block entities (chests, signs) later */

    int32_t viewers = 0; /* players with this loaded; unload at 0 */
    bool    dirty   = false; /* for saving, once you save */
};
```

- **Coordinates:** world → chunk with `x >> 4` and `x & 15` (not `/` and `%`)
  so negatives work. Section index: `(y - min_y) >> 4`.
- **Serializing:** per section, `non_air_count`, then a paletted container
  for blocks, then one for biomes. Start with two encodings:
  - *Single-valued* (0 bits per entry, one ID): uniform sections are a few
    bytes.
  - *Direct* (full bits per entry, no palette): everything else.

  Indirect palettes are a later size optimization.
- **Heightmaps:** included in the packet; compute in the generator and update
  in `set_block`. Constant for a flat world.
- **Light:** unlit chunks render black; full sky light everywhere is the easy
  start.
- **`viewers`:** incremented/decremented by `on_player_moved` as
  `loaded_chunks` changes; world frees the chunk at 0.
- Check the exact Chunk Data byte layout for your protocol version first:
  heightmaps and light fields have changed format more than once recently.

## Bugs spotted in `server::start` (as of commit 079c6cc)

- `src/server.cpp:27`: after erasing the listening pollfd, `pollfds.size()`
  equals `connections.size()`, so `i < pollfds.size() - 1` skips the last
  connection, and with zero connections `size() - 1` wraps to `SIZE_MAX`
  (out-of-bounds read). Loop to `pollfds.size()`.
- `src/server.cpp:114`: `pollfds.erase(pollfds.begin() + i + 1)` still assumes
  the listening socket is at index 0; erase at `begin() + i`.
