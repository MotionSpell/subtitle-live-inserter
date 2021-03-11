# Subtitle Live Inserter

This is the main repository of the Subtitle Live Inserter, a tool to insert subtitles "over the top" to any existing live OTT video streaming service.

## Build

You need a C++14 compiler and GNU Make. Dependencies (e.g. Signals extra) should give you guidance about thei prerequisites.

Initially:
```
git clone --recursive git@git.gpac-licensing.com:IRT/subtitle-live-inserter.git
cd subtitle-live-inserter
cd signals && PREFIX=/opt/msprod/extra ./extra.sh && cd -
EXTRA=/opt/msprod/extra make -j
```

At each build:
```
make -j
```

TODO: add general ./check script (pre-commit)

## Command-line examples

The tools comes with a usage.

When no subtitle input is set, some synthetic content (showing timecodes) will be emitted.

```
-g 4 -o romain.mpd  http://livesim.dashif.org/livesim/testpic_2s/Manifest.mpd
```

## Interactive shell

As of version 18, use the ```-i``` or ```--shell``` to enable the interactive shell.

When enabled type 'help' in the terminal to list the available commands. The goal is to dynamically control the tool with minimal impact on the output session.

## Technical details

A changelog is available.

The tool processes some pre-packaged TTML subtitles. These are then packaged.

The tool retrieves the DASH manifest. It then modifies the manifest to inject the subtitle content from the previous paragraph.

The DASH manifest modification also contains some other subtle modifications.

Overall content is modified as minimally as possible. The transformation goes from XML to XML so formatting may change, but no semantic check is applied.

## Testing

The code is covered by unit test but not by functional tests. The deployment has historically been controled by IRT (mirror at https://git.gpac-licensing.com/IRT/live-ut-workflow).

