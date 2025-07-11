# About Zucchini

A **4-bit ADPCM encoder** for the **JSystem** engine, used by a bunch of GC and Wii games, such as **Super Mario Galaxy**.

This tool can **convert standard 16-bit PCM WAV** files into raw **ADPCM4** files or **AST** (Aram Stream, streamed music format) files, using a high precision **4-bit ADPCM** encoding algorithm with up to **6 channels**!

This is an ADPCM encoder for the JSystem engine, which is **extremely accurate** when encoding, **precise**, fully supports **looping** and **loop point adjustments**, does **not decrease the loudness** of the output wave data and fully **prevents unwanted clicks or static noises** (even on console!).

<br>
<br>

## Usage

You can **drag'n'drop** standard 16-bit PCM **WAV files** onto the executable to convert it to **AST**.<br>
The output file will be **looped** with the **LoopStartSample** being **Null** and the **LoopEndSample** being the **number of samples in the track**.

You can also **drag'n'drop** an **AST** file onto the executable to **convert it to ADPCM4 encoded AST** files. It will carry over metadata, like Samplerate, channel count, IsLooped, LoopStart and LoopEnd.<br>
So feel free to use a **different AST converter** to first convert your **MP3 or WAV to AST**, and then drag'n'drop that AST file onto the **Zucchini** executable to **lower the filesize** of the AST.<br>
The output file will be **created in the same directory** with the same name, except that it adds an **.ADPCM.ast** to the end.


### Command prompt usage:

```
Zucchini <action> <in file> <out file> [loop start] [loop end]
```

Options with [brackets] around them are optional, but are necessary to be entered in the right order.

#### Action
| enc   | dec  | ast  | astconv |
| ---- | ---- | ---- | --- |
| **Encodes standard 16-bit PCM WAV files to raw 4-bit ADPCM.** | **Decode raw 4-bit ADPCM data to raw little-endian 16-bit PCM.** | **Encode and convert the given WAV file to 4-bit ADPCM encoded AST audio streams.** | **Convert a PCM16 AST to ADPCM4 AST file.** |
| Make sure to give loop point information if your sample is supposed to loop, otherwise it won't get loop-point optimizations. It will also tell you the loop point **Last and Penultimate**, in case you need those. | The decoding algorithm is 1:1 with the one used by Nintendo's games, so the decoded samples are as close as it gets to the hardware decoding process! | Make sure to enter your loop points, otherwise the output file **won't be looped!** | Allows you to convert a PCM16 encoded AST file to the much smaller ADPCM4 encoded AST file. Carries over Samplerate, LoopStartSample, LoopEndSample. Zucchini will let you know, if your AST is already ADPCM4 encoded. |

**NOTICE:** When giving loop point information, take note of the CMD output, as it will tell you that the **loop points have been shifted**, in order to have proper looping points. If you need the loop point information then take notice of it, as the CMD output will be what is actually used in the output file.

#### Usage examples:

```
Zucchini enc "in.wav" "out.adpcm"
Zucchini enc "in.wav" "out.adpcm" 0 188884
Zucchini enc "in.wav" "out.adpcm" 13355 577766

Zucchini dec "in.adpcm" "out.raw"

Zucchini ast "in.wav" "out.ast"
Zucchini ast "in.wav" "out.ast" 0 122343
Zucchini ast "in.wav" "out.ast" 235554 34555334

Zucchini astconv "in.ast" "out.ast"
```

<br>
<br>

## Building
Install a **GNU Compiler Collection** on your System and run the **makefile** using ``make``.<br>
No external libraries are required.


<br>
<br>

## Notes
Feel free to use the **encoding** / **decoding** algorithm in your own tools and converters!<br>
They lack documentation but they are pretty straight forward.

If you run into errors or other issues that you can't resolve then **open an Issue** on this repo and see if me or anyone else can help you out.

<br>
<br>

## Credits

Thanks a bunch to [XAYRGA](https://github.com/XAYRGA) for inspiring me and teaching me the basics of how the ADPCM structure works (and also for pointing out that Wind Waker has a CPU DecodeADPCM function).

<br>
<br>

## License
For License information refer to the LICENSE file on this github repo.
