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

import os


# Main build environment.
env = Environment()


# Set up build environment.
env.Append(
    CXXFLAGS=[
        '-std=c++0x',
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
  env.ParseConfig(
      'pkg-config --cflags --libs %s' % (lib))


# Main program.
env.Program(
    'basic_wm',
    Glob('*.cpp'))

