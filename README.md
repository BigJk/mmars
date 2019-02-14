```
    _____ ___  ____ ___  ____ ___________
  __/ __ `__ \/ __ `__ \/ __ `/ ___/ ___/
 __/ / / / / / / / / / / /_/ / /  (__  ) 
 _/_/ /_/ /_/_/ /_/ /_/\__,_/_/  /____/  by BigJk

```

----

**M(odern)MARS** is yet another CoreWar simulator. It's goal is not to outperform other implementations. Instead it should be compact, clean and easily embeddable into other projects (evolvers etc.) without much hassle. All functions are commented and a simple CLI is provided to use mmars as standalone software.

**Note:** P-Space is not yet implemented.

## Features
- Simple and clean MARS implementation
- Load file parser
- Multi-Threaded benchmark implementation

## CLI

```
    _____ ___  ____ ___  ____ ___________
  __/ __ `__ \/ __ `__ \/ __ `/ ___/ ___/
 __/ / / / / / / / / / / /_/ / /  (__  )
 _/_/ /_/ /_/_/ /_/ /_/\__,_/_/  /____/  by BigJk

mmars - c++ modern mars
Usage: C:\...\mmars.exe [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -w,--w,--warrior TEXT ...   a list of warrior paths
  -s,--s,--core_size INT      Core size
  -c,--c,--max_cycle INT      Maximum cycles
  -p,--p,--max_process INT    Maximum processes
  -l,--l,--max_length INT     Maximum length
  -d,--d,--min_separation INT Minimum separation
  -r,--r,--rounds INT         Rounds to fight
  -f,--f,--fixed_pos INT      Fixed position for the first round (will also be used as seed)
  --rl,--read_limit INT       Read limit (defaults to core size)
  --wl,--write_limit INT      Write limit (defaults to core size)
  -b,--b,--bench_path TEXT    The path to a folder that contains the warriors to benchmark against
  -t,--t,--bench_threads INT  The amount of threads to use for the benchmark
```

## Credits & Reference
- http://corewar.co.uk/standards/icws94.htm
- https://github.com/rodrigosetti/corewar/
- http://vyznev.net/corewar/guide.html
- http://beej.us/pizza/koth/pmars.txt
- http://www.corewar.co.uk/ankerl/exmars.htm

## Used Libraries

### CLI11
```
CLI11 1.7 Copyright (c) 2017-2019 University of Cincinnati, developed by Henry
Schreiner under NSF AWARD 1414736. All rights reserved.

Redistribution and use in source and binary forms of CLI11, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
