
= Unfact: Toolkit for Unveiling The Fact =

Unfact is a small library to intrusive, coarse-grained profiling.
Currently, unfact supports two types of profiling: 
heap profiling and timing profiling.

= Dependency =

unfact depends standard C library, but does NOT depend standard C++ library. 
For multi-threading support, unfact use win32 API and POSIX thread API 
on MS Windows and UNIX respectively. 

Unfact also uses 'atomic_ops' library, that is contained in this distribution (unser srclib/)
original version of atomic_ops library is available as a part of Boehm-Demers-Weiser Garbage collector. 
that is available at http://sourceforge.net/projects/bdwgc/.

If you use unfact in the platform other than win32 or UNIX-like systems, 
you need to implement small porting layer for your own.
