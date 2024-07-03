import logging
import os


# Supported build systems.
ENVIRONMENTS = ('gcc', 'clang',)
AddOption(
    '--build_with',
    choices=ENVIRONMENTS,
    default='gcc',
    help='Build environment.')
assert GetOption('build_with') in ENVIRONMENTS


# Common build environment.
env = Environment()
env.Append(
    CXXFLAGS=[
        '-std=c++1y',
        '-Wall',
        '-g',
    ],
    ENV={
        # For running commands.
        'PATH': os.environ.get('PATH', ''),
        # Enables color output from Clang++.
        'TERM': os.environ.get('TERM', ''),
    })
LIBS = [
    'libglog',
    'x11',
]
for lib in LIBS:
  env.ParseConfig('pkg-config --cflags --libs %s' % (lib))

# Additional build flags for Clang.
if GetOption('build_with') == 'gcc':
  pass
elif GetOption('build_with') == 'clang':
  env.Replace(
      CXX='clang++')
  env.Append(
      CXXFLAGS=[
          '-stdlib=libc++',
      ],
      LINKFLAGS=[
          '-stdlib=libc++',
      ],
      LIBS=[
          '-lc++abi',
      ],
  )
else:
  logging.fatal('Unsupported build environment \'%s\'', GetOption('build_with'))

# Main program.
env.Program(
    'basic_wm',
    Glob('*.cpp'))

