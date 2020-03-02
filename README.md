# eNoseAnnotator

eNoseAnnotator is a program used to view, annotate and store measurements of the eNose sensor. Currently Windows and Linux systems are supported.

## How to install

### Windows:
Download the windows archive from the assets of the latest release (green label).
Unpack the zip file where you want to use the program. Go into the the directory created and double-click "eNoseAnnotator.exe" in order to run the program.
If you encounter an error message about missing .dll files, you need install the vc redistributable package by running "vc_redist.x64.exe", which is located in the same directory.

### Linux:
Download the linux archive from the assets of the latest release (green label). Extract the zip file where you want to use the program.

Make the AppImage in the extracted directory executable: 
- In Nautilus (default ubuntu file browser): Right-click on the "eNoseAnnotator-\*.AppImage" and select "Properties". Under "Permissions" check "Allow executing file as program".
- In terminal: Open terminal in the directory of the AppImage. Execute `chmod +x eNoseAnnotator-*.AppImage`.

You can now run eNoseAnnotator by double-clicking the AppImage or executing `./eNoseAnnotator-*.AppImage` in the terminal.

## How to use

### Connecting a sensor

Plug in a eNose sensor. In the tool bar click the USB connection symbol and select the usb-port of the sensor. 

To start the measurement press start symbol in the tool bar. If you have not set the functionalisation of the sensor, you will be promted to do so. After calculating the base vector of the sensor the measurement will be started. The base vector is the average vector of the first few measurements.

You can pause and stop the measurement. After stopping you can start a new measurement by pressing the start icon again. If an error occurs during a measurement, you can reconnect the sensor by clicking the reconnect symbol in order to resume the measurement. 

### Annotating data

The graph in the top left shows the the relative deviation of the measurements to the base vector. You can changing the range of the x-axis of the graph by dragging & dropping the graph left and right. It is also possible to zoom in and out using the mouse wheel. 

You can select data by pressing "Ctrl" and drawing a selection rectangle in the graph. After selection data the graph on the bottom left will show the average vector of the selection. 

In order to select a class for the selected data press "Annotation->Set class of selection...". The class of a selection always consists of a class name and its abreviation. The abreviation is used to label the data in the measurement graph and should therefore be as short as possible.

### Loading/ Saving data

You can load and save data using "Measurement->Load..."/ "Measurement->Save" in the menu. You can save the whole measurement data as well as the current selection. 

The data is stored in the format of a .csv file.

### Using classifiers

The vectors of a measurement can be classified using [TorchScript](https://pytorch.org/tutorials/advanced/cpp_export.html) models. The output of the model is assumed to be a vector of the class logits. In order to interpret the model in the right way, some variables have to be part the model, while others are optional.

Necessary variables:
- classList (list of strings): list of the class name strings, have to be in the same order as the output vector

Optional:
- name (string): string of the classifier name
- N (integer): number of inputs of the model
- M (integer): number of outputs of the model
- isInputAbsolute (bool): true if absolute vectors should be used, otherwise relative vectors are used as input. Assumed to be false if not set.
- mean (list of doubles): mean for each input, should be set if the model's training set was normalised
- variance (list of doubles): variance for each input, should be set if the model's training set was normalised
- preset_name (string): name of the sensor's functionalisation preset 
