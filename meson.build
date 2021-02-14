project('otbm', 'cpp',
    license: [ 'mit' ],
    default_options: [ 'cpp_std=c++17' ]
)

headers = files('itemtype.h', 'otb.h', 'otbi.h', 'stream.h')
sources = files('otb.cpp', 'otbi.cpp', 'stream.cpp')

boost = dependency('boost', modules : ['iostreams'])
fmt = dependency('fmt')

run_target('format',
    command: ['clang-format', '-i', '-style=file', headers, sources]
)

otb = library('otb', sources, dependencies : [boost, fmt])
