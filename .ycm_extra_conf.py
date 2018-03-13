import os

def FlagsForFile( filename, **kwargs ):
    lab_path = os.path.dirname(os.path.realpath(__file__))
    return {
    'flags': [ '-I', f'{lab_path}/ext', '-I', f'{lab_path}/ext/glad/include',
        '-std=c++0x', '-Wall',  '-pedantic' ],
    }
