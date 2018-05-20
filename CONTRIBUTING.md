# Contributing to OIS

Hi there! Thanks for your interest on OIS, the Object-Oriented Input System.

For basic information about what OIS is, and it's licensing, don't hesitate to consult the README and LICENCE file in the root of the source tree.

If you are reading this, I suppose that you are interested about helping us. We are interested in both code contribution, and any eventual bug report report or feature request you could submit to us.

For simplicity and traceability, we are accepting theses contribution with the tools that are provided by GitHub (Issues and Pull requests)

## Contributing code

If you are a C++ developper, this may interest you. The goal of OIS is to be able to get inputs into a graphical application in a cross-platform manner.

The library exposes a platform-agnostic public API, behind the scene, there are multiple "backend" for different operating system/platform specific input method. 

The same "user" code will be able to get inputs from, let's say, a keyboard `key press` event with the same code on Windows and Linux.

To acheive this, every single "backend" for every single supported operating system and/or input system needs to be implemented inside OIS.

If you are familiar with a particular platform, and see that OIS doesn't support it yet, feel free to contribute and open a pull request, even if you are just starting out on it and it's still work in progress.

## Reporting a bug or suggesting a Feature

Bugs and Feature requetst are handeled via GitHub's issues tracker. There are templates that you can follow to open an issue that will ask you to fill in some information that we are going to ask you anyway if you are submiting a bug report.

Same thing goes to feature request, there's a template that will help you frame and express any particular problem or suggestion you have. 

If you just want to ask a question, you can obiously click on the `Open a regular issue` link in the "new issue page".

## Contacting the team

The best way to get in touch with somebody working on OIS is to open an issue directy on GitHub :wink:

