import sys

from setuptools import setup


assert sys.version_info.major == 3 and sys.version_info.minor >= 6, \
    "This library only supports Python 3.6 and above."


setup(
    name='pyphoxi',
    version='0.1',
    description='Python Wrapper for the PhoXi Structured Light Sensor',
    author='Kevin',
    install_requires=[
        'numpy>=1.0.0,<2.0.0',
    ],
)
