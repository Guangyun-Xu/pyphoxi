# pyphoxi

This is a simple C++/Python TCP communication module for the PhotoNeo PhoXi Structured Light Sensors. It consists of:

* C++ code that fetches data from a PhoXi Sensor and hosts a TCP server.
* Python client code that fetches data from the TCP server.

<p align="center">
 <img src="./assets/feed.png" width="600px" alt="Drawing">
</p>

## Example

Execute `PhoXiControl` then fire up the TCP server:

```bash
cd scripts
./run.sh
```

Vizualize an RGB-D feed in `opencv`:

```python
python view_feed.py 127.0.0.1 50200
```

## Installation

Run the instructions [here](https://github.com/kevinzakka/pyphoxi/blob/master/INSTRUCTIONS.md).

## References

- [Andy Zeng's RealSense Code](https://github.com/andyzeng/visual-pushing-grasping/tree/master/realsense)