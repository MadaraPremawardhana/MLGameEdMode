# ML Game EdMode

An Unreal Engine editor mode + runtime framework for wiring machine-learning
models directly into your actors and levels — no separate ML server, no
external process. Models run in-engine via Unreal's built-in **Neural
Network Engine (NNE)**, using models you export to **ONNX** from whatever you
trained them in (PyTorch, TensorFlow, scikit-learn, etc.).

It ships as a single plugin folder you drop into your project's `Plugins/`
folder. Requires **Unreal Engine 5.3+** (uses NNE, which shipped in 5.3).

## What's in the box

- **`UMLModelAsset`** — a designer-friendly wrapper around an imported ONNX
  model: names your model's inputs/outputs so the rest of the plugin (and
  your own Blueprints) can talk to it by name instead of raw tensor indices.
- **`UMLBrainComponent`** — a generic, reusable "ML brain" actor component:
  set named input features, run inference, read named output features. The
  building block for almost any gameplay ML use case.
- **`UMLGameBlueprintLibrary::RunModelOnce`** — a stateless Blueprint node for
  calling a model from anywhere (GameMode, widgets, AnimNotifies, ...).
- **Two ready-to-use example integrations** (see below): ML-driven
  procedural animation, and ML-driven procedural environment generation.
- **The "ML Game" editor mode** — a panel in the level editor's mode toolbar
  for spawning generation volumes and batch-running/clearing them across a
  level.

## Installation

1. Copy the entire `MLGameEdMode` folder (this one) into your project's
   `Plugins/` folder, so you end up with `YourProject/Plugins/MLGameEdMode/...`.
2. Open your `.uproject`, or launch the editor and let it prompt you to
   rebuild modules. If your project is Blueprint-only, the editor will offer
   to convert it to a C++ project automatically the first time you launch
   with a C++ plugin installed — accept that prompt (or run
   `GenerateProjectFiles`/regenerate and build from your IDE if you already
   have a C++ project).
3. In the editor: **Edit ▸ Plugins**, search "ML Game", confirm it's enabled,
   and also enable **NNE Runtime ORT** (and/or **NNE Runtime Basic Cpu**) if
   they aren't already — these are the actual model-execution backends NNE
   uses. Restart the editor if prompted.
4. Confirm the plugin is active: the level editor's mode toolbar (top-left of
   the viewport, next to Select/Landscape/Foliage/etc.) should now show an
   **ML Game** entry.

## Creating a model asset

1. Train your model and export it to **ONNX** (opset 9–17 is the safe range).
2. Drag the `.onnx` file into the Content Browser — the NNE plugin imports it
   automatically as a `UNNEModelData` asset.
3. Content Browser ▸ right click ▸ **Miscellaneous ▸ Data Asset** ▸ class
   `MLModelAsset`. Open it and set:
   - **Model Data** → the imported `UNNEModelData` asset.
   - **Input Feature Names** / **Output Feature Names** → one `FName` per
     input/output float, **in the exact order your model's tensors expect
     them**. This is the mapping between "the model's 12th input float" and
     a name like `DistanceToPlayer` that the rest of the plugin uses.
   - **Runtime Name** → leave as `NNERuntimeORTCpu` unless you've set up a
     different NNE runtime.

Every use case below is just "point something at a `UMLModelAsset` and feed
it named floats."

## Use case 1 (built-in): ML-driven procedural animation

`UMLProceduralAnimComponent` (a `UMLBrainComponent` subclass) drops onto any
`Character`/`Pawn` and automatically feeds the model a standard set of motion
features every tick — `Speed`, `VerticalSpeed`, `AccelerationX/Y`,
`IsFalling`, `IsCrouching`, `AimOffsetYaw` — then runs inference.

Pair it with **`UMLAnimInstance`** (make it your Animation Blueprint's parent
class) to get the model's outputs available directly as AnimGraph variables —
`MLLeanAngle`, `MLFootIK_L`, `MLFootIK_R`, `MLBlendAlpha` — with zero
Blueprint scripting: just drag them into the AnimGraph (drive a Modify Bone
node, a Layered Blend, a Lean/Aim Offset, etc.). Any output name your model
doesn't produce just reads back as 0.

Good for: foot IK / ground-adaptive locomotion, procedural lean and
counter-rotation, learned secondary motion, blending locomotion poses based
on a trained "gait" model instead of hand-authored blend spaces.

## Use case 2 (built-in): ML-driven procedural environment generation

**`AMLGenerationVolume`** — place it in a level, size its box over the area
you want populated, assign a `UMLModelAsset` and an **Asset Palette** (a list
of actor classes: rocks, trees, enemy spawners, loot pickups, ...). Its
**Generate()** function walks a grid over the box, runs the model once per
cell with that cell's normalized position (`NormX`/`NormY`, plus anything
else you add to `Extra Input Features` — a biome id, a noise value, a seed)
and reads the model's output as one score per palette entry. The winning
entry (softmax + weighted-random by default, or plain argmax) spawns
directly into the level, optionally traced down onto ground geometry.

`Generate()` / `Clear()` are `CallInEditor` — they show up as buttons on the
actor's own Details panel — and the **ML Game editor mode** toolkit can
batch-run them across every selected volume at once, or spawn new volumes
for you with a default model pre-assigned.

Good for: biome-aware asset scattering, learned dungeon/room layout
selection, difficulty-aware enemy/loot placement across a region, any
"paint a grid of decisions" generation task where you'd rather train a model
on examples than hand-write placement rules.

## Other use cases (build with `UMLBrainComponent` / the Blueprint library)

These don't need a bespoke class — the generic brain component and
`RunModelOnce` node cover them:

- **NPC behaviour / decision-making** — feed in distance-to-player, health,
  ammo, line-of-sight; read out per-action scores (attack / flee / take
  cover / patrol) via `GetTopOutputFeature()` and act on the winner.
- **Dynamic difficulty adjustment** — feed in recent deaths, time-to-kill,
  accuracy; read out a difficulty multiplier and apply it to enemy stats or
  spawn rates from your GameMode.
- **Anomaly / cheat detection** — feed in a rolling window of player speed,
  acceleration, input timing; read out an anomaly score and flag/log players
  who cross a threshold.
- **Player behaviour prediction / churn / matchmaking signals** — feed in
  session stats; read out a predicted engagement or skill score to drive
  analytics or adaptive content.
- **Audio-reactive or vision-based gameplay** — anything you can reduce to a
  fixed-size float vector in and out is fair game (RunModelOnce doesn't care
  where the numbers came from).

## Folder structure

```
MLGameEdMode/
  MLGameEdMode.uplugin
  Resources/Icon128.png
  Source/
    MLGameRuntime/              (Runtime module - ships in packaged games)
      MLModelAsset, MLModelRunner, MLBrainComponent, MLGameBlueprintLibrary,
      MLProceduralAnimComponent, MLAnimInstance, MLGenerationVolume
    MLGameEdMode/                (Editor module - editor only)
      MLGameEdModeModule, MLGameEdMode, MLGameEdModeToolkit, MLGameEdModeSettings
```

## Notes, limitations, troubleshooting

- **NNE API surface changes slightly between engine minor versions.** All NNE
  calls are isolated to one file — `Source/MLGameRuntime/Private/MLModelRunner.cpp`
  — specifically so that if your engine's exact NNE headers differ (a
  renamed method, a changed return type), that's the only file you should
  need to patch. Everything else in the plugin talks to models only through
  `FMLModelRunner`'s `LoadModel`/`RunInference` interface.
- Inference currently runs **synchronously on CPU** (`RunSync`) for
  simplicity and zero extra setup. For expensive models called every frame
  on many actors, consider batching calls, lowering `InferenceIntervalSeconds`
  on `UMLBrainComponent`, or extending `FMLModelRunner` to an async/GPU path.
  A single flat input tensor of shape `[1, N]` is assumed; if your model
  takes a different input layout, `MLModelRunner.cpp` is where to adjust it.
  A model with zero outputs configured, or a mismatched
  input/output feature count vs. what your ONNX file actually expects, will
  log a clear error to the Output Log under the `LogMLGame` category rather
  than failing silently — check there first if inference isn't producing
  results.
- The editor mode is built on the classic `FEdMode` API (still fully
  supported in UE 5.3+), chosen over the newer `UEdMode`/Interactive Tools
  Framework path for a simpler, more stable "panel with buttons" mode. If you
  want to add interactive viewport tools (click-to-paint generation, gizmos,
  etc.), that's the natural place to extend into the Interactive Tools
  Framework.
- `AMLGenerationVolume` spawns real actors into the level and tags/attaches
  them to itself for cleanup — `Clear()` (or a fresh `Generate()`, which
  clears first) removes everything it previously spawned.
