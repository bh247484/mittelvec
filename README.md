# MittelVec
## What is MittelVec?

MittelVec is a simple, single header game audio middleware backed by [miniaudio](https://github.com/mackron/miniaudio).

It uses miniaudio as an audio backend but manages its own audio buffers internally. MittelVec simply uses miniaudio as a cross platform compatible way to read wav files and write into system level audio hardware buffer outputs.

MittelVec has a modular audio graph with an AudioNode base class that's extended by envelope, filter, gain, noise generator, pitch shift, and sampler nodes. Those nodes are then rolled up into game audio specific abstractions that manage and trigger sound FX (SamplePack) and music cues (MusicCueOrchestrator).

## Why did I build this?

In a [previous project](https://github.com/bh247484/wwise-quake3-integration/pull/1) I integrated [Wwise](https://www.audiokinetic.com/en/wwise/) into a legacy Quake build system. It was a painful experience. So for future projects I wanted a simple, portable way to quickly program custom game audio in alien codebases. A single header file library seemed a natural choice.

## Where can I see MittelVec in action?

[link to snes hack project]...

## How can I use MittelVec in my own projects?

I've generated the single header file and committed it to the repo at `dist/mittelvec.h`. If you want to use it you can simply include that header file in your project alongside miniaudio.h (place both in the same dir) and you're good to go. If you'd like to extend or modify the source code and/or add your own custom nodes you can do so and then run the `generate_single_header.py` script in the root of the repo to regenerate the new single header file. The script will overwrite the existing file in the `dist/` dir.

The `generate_single_header.py` file is a bit brittle and if I could do things over again I would consider declaring and implementing everything inline in a single header file from the start. Although that comes with its own tradeoffs... In any case the `generate_single_header.py` script works well enough for now and will likely work for you too, assuming your changes to the source code are not too broadband.

## Any possibility of future development?

I've put the project on ice for now but would like to revisit it eventually. In particular I'd like to refactor the Sampler implementation. I programmed myself into a corner there and hamstrung myself with some upstream architectural choices. That meant the memory management and logic is a little counter intuitive. I'm sure it could be simplified and improved.

I'd also like to add a third top level abstraction alongside Sampler and MusicCueOrchestrator that exposes some synthesizer primitives so that you could parameterize a synthesizer dynamically with game engine outputs. LFOs and other modulation sources as well as programmatic randomization for the sythesizer and other node parameters would add some creative expression.

The code as it stands was designed to take static parameters at compile time. I'd like to update it to accepts streaming values at run time. I think reimagining threading would be a prerequisite for that as well.

For now though MittelVec solves what I set out to solve.