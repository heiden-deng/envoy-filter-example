# Envoy filter example

This project demonstrates the linking of additional filters with the Envoy binary.
A new filter `echo2` is introduced, identical modulo renaming to the existing
[`echo`](https://github.com/envoyproxy/envoy/blob/master/source/extensions/filters/network/echo/echo.h)
filter. Integration tests demonstrating the filter's end-to-end behavior are
also provided.

For an example of additional HTTP filters, see [here](http-filter-example).

## Building

To build the Envoy static binary:

1. `git submodule update --init`

2.  To build Network filter  
`bazel build  -c dbg --copt "-DENVOY_IGNORE_GLIBCXX_USE_CXX11_ABI_ERROR=1"  //:envoy`

3. To build http filter  
`bazel build  -c dbg --copt "-DENVOY_IGNORE_GLIBCXX_USE_CXX11_ABI_ERROR=1"  //http-filter-example:envoy `

## Testing

To run the `echo2` integration test:

`bazel test //:echo2_integration_test`

To run the regular Envoy tests from this project:

`bazel test @envoy//test/...`

## How it works

The [Envoy repository](https://github.com/envoyproxy/envoy/) is provided as a submodule.
The [`WORKSPACE`](WORKSPACE) file maps the `@envoy` repository to this local path.

The [`BUILD`](BUILD) file introduces a new Envoy static binary target, `envoy`,
that links together the new filter and `@envoy//source/exe:envoy_main_lib`. The
`echo2` filter registers itself during the static initialization phase of the
Envoy binary as a new filter.
