"""Disposable route-design experiment; does not alter the native game.

Generate resolved tile geometry, not a choice between stored map templates.
The SVGs are topology diagrams, not proposed terrain art or game screens.
"""

import argparse
import collections
import json
import random
from pathlib import Path

WIDTH, HEIGHT = 20, 11
ROLES = ("Camp", "Moss bend", "Relay", "Stone shelf", "Sealed cache")


def corridor(start, end, vertical_first):
    x, y = start
    end_x, end_y = end
    cells = [(x, y)]
    while (x, y) != end:
        if (vertical_first and y != end_y) or x == end_x:
            y += 1 if y < end_y else -1
        else:
            x += 1 if x < end_x else -1
        cells.append((x, y))
    return cells


def distance(cells, start, end):
    queue = collections.deque([(start, 0)])
    seen = {start}
    while queue:
        (x, y), steps = queue.popleft()
        if (x, y) == end:
            return steps
        for neighbor in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if neighbor in cells and neighbor not in seen:
                seen.add(neighbor)
                queue.append((neighbor, steps + 1))
    return None


def generate(seed):
    randomizer = random.Random(seed)
    # Reserve the upper rows for a genuinely concealed sample connector.
    sites = [(randomizer.randrange(2, 18), 9),
             (randomizer.randrange(2, 18), 4)]
    for _ in range(2):
        candidates = [(x, y) for y in range(5, 9) for x in range(2, 18)
                      if all(abs(x - sx) + abs(y - sy) >= 4 for sx, sy in sites)]
        sites.append(randomizer.choice(candidates))
    sites.append((randomizer.randrange(2, 18), 1))

    # A shuffled connected tree varies adjacency as well as role placement.
    order = list(range(4))
    randomizer.shuffle(order)
    edges = []
    for index in range(1, len(order)):
        edges.append((order[index], randomizer.choice(order[:index])))
    if randomizer.randrange(2):
        unused = [(a, b) for a in range(4) for b in range(a + 1, 4)
                  if (a, b) not in edges and (b, a) not in edges]
        edges.append(randomizer.choice(unused))
    visible = set()
    for a, b in edges:
        visible.update(corridor(sites[a], sites[b], bool(randomizer.randrange(2))))

    cache_x, cache_y = sites[4]
    lead = set(corridor(sites[1], (sites[1][0], cache_y), True))
    lead.update(corridor((sites[1][0], cache_y), (cache_x, cache_y), False))
    # Only offer the optional trail where bypassing the ordinary investigation
    # stop has concrete route value. Equal-length choices exposed by seed29
    # were a design defect, not a reason to claim every event is interesting.
    useful_event_sites = []
    for candidate in (2, 3):
        ordinary_steps = distance(visible, sites[candidate], sites[1]) + distance(visible | lead, sites[1], sites[4])
        direct_steps = abs(sites[candidate][0] - cache_x) + sites[candidate][1] - cache_y
        if ordinary_steps - direct_steps >= 4:
            useful_event_sites.append(candidate)
    event_site = randomizer.choice(useful_event_sites) if useful_event_sites else None
    # This spike may omit an event when no useful placement exists; it must not
    # move a sample or make the ordinary starter route depend on the event.
    if event_site is None:
        event = set()
    else:
        event = set(corridor(sites[event_site], (sites[event_site][0], 2), True))
        event.update(corridor((sites[event_site][0], 2), (cache_x, 2), False))
        event.update(corridor((cache_x, 2), sites[4], True))

    assert sites[4] not in visible
    assert all(distance(visible, sites[0], site) is not None for site in sites[:4])
    assert distance(visible | lead, sites[0], sites[4]) is not None
    if event_site is not None:
        assert distance(visible | event, sites[event_site], sites[4]) is not None
    assert all(0 <= x < WIDTH and 0 <= y < HEIGHT for x, y in visible | lead | event)
    return {
        "status": "provisional geometry and event-design experiment; not game runtime",
        "geometry_version": "route-design-trial-1",
        "seed": seed,
        "grid": [WIDTH, HEIGHT],
        "sites": [{"role": role, "tile": list(tile)} for role, tile in zip(ROLES, sites)],
        "construction_edges": [list(edge) for edge in edges],
        "visible_tiles": [list(tile) for tile in sorted(visible)],
        "ordinary_trace_tiles": [list(tile) for tile in sorted(lead - visible)],
        "event_connector_tiles": [list(tile) for tile in sorted(event - visible)],
        "event": None if event_site is None else {
            "site": ROLES[event_site],
            "choices": ["Recover 3 Energy", "Read the trail"],
            "supplies": [0, 3, 0],
            "ordinary_sample_steps_from_event": distance(visible, sites[event_site], sites[1]) + distance(visible | lead, sites[1], sites[4]),
            "event_sample_steps_from_event": distance(visible | event, sites[event_site], sites[4]),
            "saved_once": True,
            "reveal": "connector and sealed cache location only; no sample contents",
        },
    }


def diagram(result):
    scale, left, top = 26, 28, 86
    def center(tile):
        return left + tile[0] * scale + scale // 2, top + tile[1] * scale + scale // 2
    parts = ['<svg xmlns="http://www.w3.org/2000/svg" width="576" height="452" viewBox="0 0 576 452">',
             '<rect width="576" height="452" fill="#18252d"/>',
             '<g font-family="sans-serif" fill="#edf4ee">',
             f'<text x="28" y="30" font-size="19">Generated route · seed {result["seed"]}</text>',
             '<text x="28" y="53" font-size="12">Topology diagram · all connectors shown for review</text>',
             '<text x="28" y="73" font-size="12">Start/player: Camp · movement awards nothing</text>']
    for y in range(HEIGHT):
        for x in range(WIDTH):
            parts.append(f'<rect x="{left+x*scale}" y="{top+y*scale}" width="26" height="26" fill="none" stroke="#30444e"/>')
    for key, color in (("visible_tiles", "#3c7385"), ("ordinary_trace_tiles", "#7ea965"), ("event_connector_tiles", "#d59753")):
        for tile in result[key]:
            x, y = center(tile)
            parts.append(f'<rect x="{x-7}" y="{y-7}" width="14" height="14" fill="{color}"/>')
    for index, site in enumerate(result["sites"]):
        x, y = center(site["tile"])
        color = "#f5ba69" if result["event"] and site["role"] == result["event"]["site"] else "#edf4ee"
        parts.append(f'<rect x="{x-10}" y="{y-10}" width="20" height="20" fill="#18252d" stroke="{color}" stroke-width="2"/>')
        parts.append(f'<text x="{x}" y="{y+4}" font-size="12" text-anchor="middle">{index}</text>')
    parts.extend(['<text x="28" y="395" font-size="12">0 Camp   1 Moss bend   2 Relay   3 Stone shelf   4 Sealed cache</text>',
                  '<text x="28" y="417" font-size="12">Blue: visible   Green: ordinary trace   Orange: event trail</text>',
                  '<text x="28" y="438" font-size="12">Review reveals hidden routes; player would discover them through actions.</text>',
                  '</g></svg>'])
    return "\n".join(parts)


def render_diagram(result, destination):
    # Rasterize the same data as a diagram using the installed document runtime.
    # This does not manipulate any source art or pretend to be a game screen.
    from PIL import Image, ImageDraw, ImageFont
    canvas = Image.new("RGB", (576, 476), "#18252d")
    draw = ImageDraw.Draw(canvas)
    font_path = Path("C:/Windows/Fonts/segoeui.ttf")
    font = ImageFont.truetype(str(font_path), 14)
    title = ImageFont.truetype(str(font_path), 22)
    draw.text((28, 15), f"Generated route · seed {result['seed']}", font=title, fill="#edf4ee")
    draw.text((28, 47), "Topology diagram · all connectors shown for review", font=font, fill="#edf4ee")
    draw.text((28, 67), "Start/player: Camp · movement awards nothing", font=font, fill="#edf4ee")
    for y in range(HEIGHT):
        for x in range(WIDTH):
            left, top = 28 + x * 26, 96 + y * 26
            draw.rectangle((left, top, left + 26, top + 26), outline="#30444e")
    for key, color in (("visible_tiles", "#3c7385"), ("ordinary_trace_tiles", "#7ea965"), ("event_connector_tiles", "#d59753")):
        for x, y in result[key]:
            left, top = 28 + x * 26, 96 + y * 26
            draw.rectangle((left + 6, top + 6, left + 20, top + 20), fill=color)
    for index, site in enumerate(result["sites"]):
        x, y = site["tile"]
        left, top = 28 + x * 26, 96 + y * 26
        color = "#f5ba69" if result["event"] and site["role"] == result["event"]["site"] else "#edf4ee"
        draw.rectangle((left + 3, top + 3, left + 23, top + 23), fill="#18252d", outline=color, width=2)
        draw.text((left + 13, top + 12), str(index), anchor="mm", font=font, fill="#edf4ee")
    for y, line in ((398, "0 Camp   1 Moss bend   2 Relay   3 Stone shelf   4 Sealed cache"),
                    (420, "Blue: visible   Green: ordinary trace   Orange: event trail"),
                    (442, "Hidden routes shown here only for review; discovered through actions.")):
        draw.text((28, y), line, font=font, fill="#edf4ee")
    canvas.save(destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seeds", nargs="+", type=int, default=[17, 29])
    parser.add_argument("--output", type=Path, default=Path(__file__).parent / "generated-trial")
    args = parser.parse_args()
    # Bounded sanity sweep answers connectivity/variation, not fun or balance.
    examples = [generate(seed) for seed in range(1, 65)]
    fingerprints = {json.dumps([item["sites"], item["visible_tiles"]], sort_keys=True) for item in examples}
    assert len(fingerprints) == 64
    args.output.mkdir(parents=True, exist_ok=True)
    for seed in args.seeds:
        result = generate(seed)
        (args.output / f"route-{seed}.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
        (args.output / f"route-{seed}.svg").write_text(diagram(result), encoding="utf-8")
        render_diagram(result, args.output / f"route-{seed}.png")
    print(json.dumps({"checked_seeds": 64, "unique_geometry_results": len(fingerprints),
                      "exported": args.seeds, "limitation": "disposable design generator; not native runtime or fun evidence"}))


if __name__ == "__main__":
    main()
