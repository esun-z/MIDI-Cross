# MIDI Cross

Retransmit MIDI messages to multi MIDI ports.

## Usage

This GUI program should be simple to use and the UI is designed to be user-friendly. Just read the description of each release to learn more.

## Using Library & Frame

RtMidi

Qt 5.15.1

## Platform

Target Platform: Win32 (default)

Change the definition in RtMidi.h and the libs in MIDICrossGUI.pro to switch to other target platforms.

Both Qt and RtMidi support multi platforms, and there is no windows API used in the codes. Thus it will be easy to switch the target platform to Mac OSX or Linux.

## Current Issue

- The RtMidi sendMessage function do not work properly when sending multi messages to a multi channel midi device in a short time, 5ms for instance. 

  I am not pretty sure if this is cased by RtMidi. However, PortMidi works properly facing this scene. Please try MidiRetransmit (Based on PortMidi) instead in this situation.

