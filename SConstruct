# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                                                             #
#    Copyright (C) 2013 Chuan Ji <jichuan89@gmail.com>                        #
#                                                                             #
#    Licensed under the Apache License, Version 2.0 (the "License");          #
#    you may not use this file except in compliance with the License.         #
#    You may obtain a copy of the License at                                  #
#                                                                             #
#     http://www.apache.org/licenses/LICENSE-2.0                              #
#                                                                             #
#    Unless required by applicable law or agreed to in writing, software      #
#    distributed under the License is distributed on an "AS IS" BASIS,        #
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
#    See the License for the specific language governing permissions and      #
#    limitations under the License.                                           #
#                                                                             #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

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

