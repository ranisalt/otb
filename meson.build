project('otbm', 'cpp',
    license: [ 'mit' ],
    default_options: [ 'cpp_std=c++17' ]
)

headers = files('otb.h')
sources = files('otb.cpp')

boost = dependency('boost', modules : ['iostreams'])

run_target('format',
    command: ['clang-format', '-i', '-style=file', headers, sources]
)

otb = library('otb', sources, dependencies : [boost])
