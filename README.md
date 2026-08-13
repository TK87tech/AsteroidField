# Asteroids

A compact, from-scratch implementation of the arcade classic in **C++17** on
**Cocos2d-x 4.0**. Every visual is generated in code with `DrawNode` primitives
and system-font labels — no image, audio, or font assets are loaded at runtime.
Built as a focused gameplay-programming sample: tight game loop, clean state
management, and frame-rate-independent physics.

## Overview

Pilot a ship through escalating waves of drifting asteroids. Thrust and rotate
to line up shots, and break large asteroids into medium and small fragments.
Clear a wave and the next, larger one spawns. Three lives; a collision destroys
the ship, which respawns with brief invulnerability. The playfield wraps like a
torus — leave one edge and reappear on the opposite side.

## Controls

| Key | Action |
|-----|--------|
| ← / → | Rotate the ship |
| ↑ | Thrust |
| Space | Fire |
| R | Restart after Game Over |

## Building

Requires a working **Cocos2d-x 4.0** installation. From the project root:

```bash
cmake -S . -B build
cmake --build build --config Release
```

Run the produced executable (in the build output directory) to play.

## Design notes

The project is deliberately small but structured the way a larger game would
be, so the moving parts stay readable:

- **Separate entity types.** `Ship`, `Asteroid`, and `Bullet` are distinct
  structs rather than one shared object, keeping each system's data and update
  logic self-contained. Asteroids carry a precomputed rough polygon for their
  outline so each rock looks distinct.
- **Frame-rate independence.** All motion, firing cooldowns, respawn delays,
  and invulnerability timers are driven by delta time, so behavior is
  consistent regardless of frame rate.
- **Progressive difficulty.** Asteroid sizes (large / medium / small) have
  their own radii, speeds, and score values; large and medium asteroids split
  into smaller ones when destroyed, and each cleared wave spawns a larger set.
- **Fair spawning.** New asteroids never appear within a safe radius of the
  ship, preventing unavoidable deaths on respawn.
- **Single-scene state machine.** Playing, ship-destroyed/respawning, and
  game-over states are handled in one scene with no reloads, and `R` performs a
  full clean reset.
- **Tunable by design.** Ship handling, bullet behavior, cooldowns, wave
  counts, and timer durations are all named constants at the top of the header,
  so the game's feel can be adjusted in one place without hunting through the
  logic.

## Rendering approach

Two `DrawNode` layers separate concerns: one redrawn every frame for the ship,
asteroids, and bullets, and one for the HUD (lives icons drawn as primitives).
Score, wave, and end-state text use system-font labels. This keeps the entire
game asset-free and self-contained in a single buildable project.

## License

Released under the MIT License. See [LICENSE](LICENSE) for details.
