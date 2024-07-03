def FlagsForFile( filename, **kwargs ):
  return {
      'flags': [
          '-x', 'c++',
          '-std=c++1y',
          '-Wall',
          '-Werror',
      ],
  }
