from setuptools import setup

with open('README.md', 'r') as f:
    sbf_long_description = f.read()

sbf_classifiers = [
    'Programming Language :: Python',
    'Programming Language :: Python :: 3',
    'Development Status :: 2 - Pre-Alpha',
    'Intended Audience :: Developers',
    'Intended Audience :: Science/Research',
    'Topic :: Software Development :: Libraries :: Python Modules',
]


setup(name='sbf',
      version='0.2.0',
      py_modules=['sbf'],
      url='http://github.com/peterspackman/sbf',
      author='Peter Spackman',
      author_email = 'peterspackman@fastmail.com',
      description='python interface for the SBF format',
      long_description=sbf_long_description,
      keywords = ['binary', 'file format', 'data'],
      classifiers=sbf_classifiers,
      entry_points={
          'console_scripts': [
              'sbfpytool=sbf:main',
          ],
      },
      license='GPLv3',
      install_requires=['numpy',
          'pathlib2;python_version<="2.7"'],
)
