import json
import os
import sys

def is_stale(inputs, outputs) -> bool:
    if not all(os.path.exists(o) for o in outputs):
        return True

    return max(os.path.getmtime(i) for i in inputs) > min(os.path.getmtime(o) for o in outputs)

def generate_block_states() -> None:
    with open("reports/blocks.json", "r") as blocks_json, open("registry/blocks.hpp", "w") as blocks_hpp:
        blocks_hpp.write(
"""
#pragma once
#include <cstdint>

namespace block {
"""
        )

        blocks = json.load(blocks_json)
        for name, data in blocks.items():
            if len(data["states"]) == 1:
                blocks_hpp.write(
f"""
    constexpr uint16_t {name[10:]} = {data["states"][0]["id"]};
"""
                )

        blocks_hpp.write(
"""
};
"""
        )

def generate_items() -> None:
    with open("reports/registries.json", "r") as registries_json, open("registry/items.hpp", "w") as items_hpp:
        items_hpp.write(
"""
#pragma once
#include <cstdint>

namespace item {
"""
        )

        items = json.load(registries_json)["minecraft:item"]["entries"]
        for name, data in items.items():
            items_hpp.write(
f"""
    constexpr int32_t {name[10:]} = {data["protocol_id"]};
"""
            )

        items_hpp.write(
"""
};
"""
        )

def generate_components() -> None:
    with open("reports/registries.json", "r") as registries_json, open("registry/components.hpp", "w") as components_hpp:
        components_hpp.write(
"""
#pragma once
#include <cstdint>

namespace component {
"""
        )

        components = json.load(registries_json)["minecraft:data_component_type"]["entries"]
        for name, data in components.items():
            components_hpp.write(
f"""
    constexpr int32_t {name[10:].replace("/", "_")} = {data["protocol_id"]};
"""
            )

        components_hpp.write(
"""
};
"""
        )

def generate_registries() -> None:
    registries = None

    with open("reports/registries.json", "r") as registries_json:
        registries = json.load(registries_json)

    by_protocol_id = lambda item: int(item[1]["protocol_id"])

    # sort inner and outer by protocol id
    registries = {
        k: {**v, "entries": dict(sorted(v["entries"].items(), key = by_protocol_id))}
        for k, v in sorted(registries.items(), key = by_protocol_id)
    }

    with open("inc/registry.hpp", "r") as registry_hpp:
        registry_hpp_lines = registry_hpp.readlines()

    with open("src/registry.cpp", "r") as registry_cpp:
        registry_cpp_lines = registry_cpp.readlines()

    begin = next(i for i, l in enumerate(registry_cpp_lines) if l.strip() == "// BEGIN GENERATED")
    end = next(i for i, l in enumerate(registry_cpp_lines) if l.strip() == "// END GENERATED")

    hand_written = sum(
        l.strip() == "registry {"
        for l in registry_cpp_lines[:begin] + registry_cpp_lines[end:]
    )

    for i in reversed(range(len(registry_hpp_lines))):
        line = registry_hpp_lines[i]
        if line.lstrip().startswith("std::array<registry"):
            indent = line[: len(line) - len(line.lstrip())]
            registry_hpp_lines[i] = f"{indent}std::array<registry, {len(registries) + hand_written}> registries;\n"
            break

    registries_array_str = ""

    for name, data in registries.items():
        registries_array_str += f"""
            registry {{
                \"{name}\",
                {{
                    {
                        ",\n                    ".join(
                            [f"{{{{\"{_name}\"}}, {{nullptr}}}}" for _name, _data in data["entries"].items()]
                        )
                    }
                }}
            }},
"""

    registry_cpp_lines[begin + 1 : end] = [registries_array_str + "\n"]

    with open("inc/registry.hpp", "w") as registry_hpp:
        registry_hpp.writelines(registry_hpp_lines)

    with open("src/registry.cpp", "w") as registry_cpp:
        registry_cpp.writelines(registry_cpp_lines)


if __name__ == "__main__":
    force = "--force" in sys.argv

    generators = [
        (generate_block_states, ["reports/blocks.json"], ["registry/blocks.hpp"]),
        (generate_items, ["reports/registries.json"], ["registry/items.hpp"]),
        (generate_components, ["reports/registries.json"], ["registry/components.hpp"]),
        (generate_registries, ["reports/registries.json"], ["inc/registry.hpp", "src/registry.cpp"]),
    ]

    for generator, inputs, outputs in generators:
        if force or is_stale(inputs + [__file__], outputs):
            print(f"{generator.__name__}: generating")
            generator()
        else:
            print(f"{generator.__name__}: up to date")

