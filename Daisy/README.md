# Owlsy FAQ

## Is it a bird, is it a flower?

Owlsy is a port of [OWL/OpenWare firmware](https://github.com/pingdynasty/OpenWare) to [Daisy](https://www.electro-smith.com/daisy) hardware

## Which devices are supported?

Currently only Daisy Patch.

## Is it an official firmware supported by Electro-Smith or Rebel Technology?

It is not, so don't expect any sort of guarantees or support from those companies.

## Which peripherals from Daisy Patch are supported?

There's no support for SD cards in the OWL firmware itself, so it can't be used for the time being. OTOH, its USB stack is in better condition. It can be used as USB MIDI device (in addition to Serial MIDI). TODO: Mention USB audio?

## How are flash storages used?

Flash storage is only used for bootloader. Firmware and patches are stored on QSPI flash chip.

## How is memory used?

* ITCM RAM - used for interrupt vectors and selected time-critical code of firmware
* DTCM RAM - stores firmware .data and .bss sections, stack and heap
* AXI (D1) SRAM - stores currently loaded user patch. This means that maximum patch size (code + data) is 512kb.
* AHB (D2) SRAM - stores firmware code that is not copied to ITCM, static data (except lookup tables) and DMA buffers (last 32kb)
* D3 SRAM is currently unused
* SDRAM chip is fully usable by user patch. It is allocated as dynamic memory.

## Where can I get binaries?

TODO: github releases

## How can I built it from source?

TODO: MidiBootDaisy, OpenWare fork.

## How can I run it?

1. Install MidiBoot port

2. Enter bootloader mode

3. Erase QSPI flash if necessary - TODO: SySex command

4. Flash firmware

5. Reboot

## How do I build patches?

TODO: OwlProgram fork. Test other targets besides C++/Faust

## Where can I see some patch examples?

Start with [tutorials](https://www.rebeltech.org/tutorials/) for Rebel Technology devices. As for code, largest collections are:

* https://www.rebeltech.org/patch-library/

* https://github.com/pingdynasty/OwlPatches

* https://github.com/pingdynasty/OwlGenPatches

* https://github.com/marsus/MyPatches

* https://github.com/olilarkin/OL-OWLPatches

## How are patches controlled?

OWL firmware supports up to 40 parameters. On Daisy Patch, the following mappings are used:

* A - D: ADC inputs
* E - F: DAC outputs
* Remaining parameters are virtual, so can be set by encoder, MIDI or from running patch

This is [full list](https://github.com/pingdynasty/OpenWare/blob/master/Source/OpenWareMidiControl.h#L9-L54) of parameter names.

You can also address gate inputs as buttons A/B, gate output as button C.

## How does Owlsy UI work?

Long encoder press brings out active parameter selection. When you're in this menu, you can turn encoder to change active encoder. Releasing it returns you to main menu.

Active parameter value can be changed by turning encoder when you're in main menu.

Short encoder press brings out settings menu. It contains multiple pages that can be scrolled by encoder. Long press or scrolling it to the end exits this menu. Current UI pages:

* *Setup* - clicking here lets you change encoder sensitivity (parmeter increment size for turning encoder in main menu).
* *Status* - shows current device stats - CPU and memory load, flash storage usage
* *Patch* - browse list of patches. Click to select previously stored patch, click again to load it.
* *Data* - browse user resources. This is static data stored on QSPI storage, so it can can contain various lookup tables, wavetables or any other data. System resources (i.e. `__SETTINGS__`) start with `__` and are write-protected. Anything else can be deleted by clicking on it.
* *Gates* - this displays current state of gate inputs/outputs. Clicking toggles output value.
* *Scope* - use this to quickly check audio signals in inputs/output. Clicking and scrolling selects different audio channel. Note that this is not very accurate - audio is sample at display refresh rate.

## I have more questions!

This port should be discussed on Daisy forum.

Questions about writing patches (unrelated to Daisy) can be asked on RebelTech forum. 

TODO: links
