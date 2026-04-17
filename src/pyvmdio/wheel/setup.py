from setuptools import setup, Distribution

class BinaryDistribution(Distribution):
    def has_ext_modules(self):
        return True

setup(
    packages=["pyvmdio", "pyvmdio._pyvmdio"],
    package_data={
        "pyvmdio": ["*.pyd", "*.pyi"],
        "pyvmdio._pyvmdio": ["*.pyi"]
    },
    distclass=BinaryDistribution,
)