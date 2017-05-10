# MicroGP v2

![Status: Obsolete](https://img.shields.io/badge/status-obsolete-red.svg)
[![License: GPL](https://img.shields.io/badge/license-gpl--2.0-green.svg)](https://opensource.org/licenses/GPL-2.0)
![Language: C](https://img.shields.io/badge/language-C-blue.svg)

MicroGP (µGP, uGP, ugp, `&micro;GP`) is a versatile optimizer able to outperform both human experts and conventional heuristics in finding the optimal solution of hard problems. Given a task, it first fosters a set of random solutions, then iteratively refines and enhance them. Its heuristic algorithm uses the result of the evaluations, together with other internal information, to efficiently explore the search space, and eventually to produce the optimal solution. MicroGP original purpose was creating assembly-language programs to test different microprocessors, hence the Greek letter micro in its name.

MicroGP v2 was developed in 2003 and maintained since 2006 (DOI: [10.1007/s10710-005-2985-x](https://link.springer.com/article/10.1007/s10710-005-2985-x)). The first version was codenamed *Chicken Pox*, because the isolation caused by that infection allowed to write most of the code in a single week. It added several new features and significantly broadened the applicability of MicroGP v1. It was able to load a list of parametric code fragments, called *macros*, and optimize their order inside a test program. With time, it has been coerced into solving problems it was not meant for. While useful for improving its performance, this extended usage made the basic limitations of the tool clear, and ultimately led to the need to [re-implement µGP from scratch](https://github.com/squillero/microgp3).

**Copyright © 2003-2006 Giovanni Squillero**

MicroGP v2 is free software: you can redistribute it and/or modify it under the terms of the [GNU General Public License](http://www.gnu.org/licenses/) as published by the *Free Software Foundation*, either [version 2](https://opensource.org/licenses/GPL-2.0) of the License, or (at your option) any later version.
