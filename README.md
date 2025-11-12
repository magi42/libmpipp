# MPICH1 C++ wrapper

Marko Grönroos (magi@iki.fi)

This is my minimalistic C++ wrapper for MPICH from 1999-2000.
*There now exists more current proper ones.*

There are a number of small applications in the `examples` directory.
Most of them are my excercise answers in the Parallel and High Performance Computing courses in 1999.

The library requires [MagiCLib++](https://github.com/magi42/magiclib).

Some examples:

## N-Body

[examples/nbody.cc](examples/nbody.cc)  
[examples/nbody.h](examples/nbody.h)  
Runs an n-body simulation in parallel.

See [examples/nbody.pdf](examples/nbody.pdf) for the exercise answer.

![Mandelbrot figure](https://raw.githubusercontent.com/magi42/libmpipp/refs/heads/main/examples/nbody-random.png)

## Mandel

[examples/mandel.cc](examples/mandel.cc)  
Generates a Mandelbrot figure.

![Mandelbrot figure](https://raw.githubusercontent.com/magi42/libmpipp/refs/heads/main/examples/mandel.gif)

## Wire and a String

[examples/wire.cc](examples/wire.cc)  
A wire simulation.

[examples/wireelement.h](examples/wireelement.h)  
[examples/wireelement.cc](examples/wireelement.cc)  
A wire simulation.

[examples/wibstring.h](examples/wibstring.h)  
[examples/wibstring.cc](examples/wibstring.cc)  
A string simulation.

![Wire](https://raw.githubusercontent.com/magi42/libmpipp/refs/heads/main/examples/wire.gif)

## Ping

[examples/ping.cc](examples/ping.cc)  
Simply pings all the nodes.

See [examples/ping.pdf](examples/ping.pdf) for the exercise answer.
