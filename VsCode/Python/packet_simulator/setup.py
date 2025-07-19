# setup.py
from setuptools import setup, find_packages

setup(
    name="packet_simulator",
    version="0.1",
    packages=find_packages(where="packet_simulator"),
    package_dir={"": "packet_simulator"},
)
