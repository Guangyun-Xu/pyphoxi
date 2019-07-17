## Compilation

0. Install the python wrapper.

```python
pip install -e .
```

1. Download `PhoXiControl` from Photoneo's [3D Scanning Software](https://www.photoneo.com/3d-scanning-software/).

2. Unpack the downloaded `.tar` file

```bash
tar -xvf PhotoneoPhoXiControlInstaller-1.2.7-Ubuntu16.tar
```

3. Execute the script.

```bash
chmod +x PhotoneoPhoXiControlInstaller-1.2.7-Ubuntu16-STABLE.run
./PhotoneoPhoXiControlInstaller-1.2.7-Ubuntu16-STABLE.run
```

Note that the `PhotoNeo` API code is located at `/opt/PhotoneoPhoXiControl/API`.

4. Restart your computer

```bash
sudo reboot
```

5. Ensure `PhoXiControl` works. This should launch Photoneo's GUI.

```bash
PhoXiControl
```

6. Compile `pyphoxi`.

```bash
git clone https://github.com/kevinzakka/pyphoxi.git
cd pyphoxi
cmake .
make
```

## Obtaining Camera Intrinsics

Run the intructions written [here](http://wiki.photoneo.com/index.php/Frequently_asked_questions_and_Frequently_experienced_difficulties#How_do_I_retrieve_the_intrinsic_parameters_of_the_scanner.3F).