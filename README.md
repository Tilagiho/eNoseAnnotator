# eNoseAnnotator

eNoseAnnotator is a program used to view, annotate and store measurements of the eNose sensor. Currently Windows and Linux systems are supported.

## How to install

### Windows:
First, install the [sensor drivers](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers).
Then download the eNoseAnnotator windows archive from the assets of the latest release (green label).
Unpack the zip file where you want to use the program. Install the vc redistributable package by running "vc_redist.x64.exe" in the directory "bin".
Finally, double-click "eNoseAnnotator.exe" in the directory "bin" to run the program.


### Linux:
Download the AppImage from the assets of the latest release (green label) and put it where you want to run the program.

Make the AppImage in the extracted directory executable: 
- In Nautilus (default ubuntu file browser): Right-click on the "eNoseAnnotator-\*.AppImage" and select "Properties". In the "Permissions" tab check the "Allow executing file as program" checkbox.
- In terminal: Open terminal in the directory of the AppImage. Execute `chmod +x eNoseAnnotator-*.AppImage`.

You can now run eNoseAnnotator by double-clicking the AppImage or executing `./eNoseAnnotator-*.AppImage` in the terminal.

## How to use

### Connecting a sensor

Plug in a eNose sensor. In the tool bar click the USB connection symbol, select the usb-port of the sensor and hit apply. 

To start the measurement press start symbol in the tool bar. If you have not set the functionalisation of the sensor, you will be promted to do so. After calculating the base vector of the sensor the measurement will be started. The base vector is the average vector of the first few measurements.

You can pause and stop the measurement. After stopping you can start a new measurement by pressing the start icon again. If an error occurs during a measurement, you can click the reconnect symbol in order to resume the measurement. 

### Annotating data

The graph in the top left shows the the relative deviation of the measurements to the base vector. You can changing the range of the x-axis of the graph by dragging & dropping the graph left and right. It is also possible to zoom in and out using the mouse wheel. 

You can select data by pressing "Ctrl" and drawing a selection rectangle in the graph. After selecting data the graph on the bottom left will show the average vector of the selection. 

In order to select a class for the selected data press "Annotation->Set class of selection...".

### Loading/ Saving data

You can load and save data using "Measurement->Load..."/ "Measurement->Save" in the menu. You can save the whole measurement data as well as the current selection. 

The data is stored in the format of a .csv file.

## Classifiers

The vectors of a measurement can be classified using [TorchScript(.pt)](https://pytorch.org/tutorials/advanced/cpp_export.html) models. 

### Training own classfiers

You can train your own classifier using pyTorch and convert it into TorchScript. The following information should be considered when doing so.

The output of the classifier can either be a vector of the class logits or class probabilities. In order to interpret the model in the right way, some variables have to be part the model, while others are optional.

| Variable        | Optional |Type             | Description                                                                                                | Default value |
| --------------- | :------: | :-------------: | ---------------------------------------------------------------------------------------------------------- | ------------- |
| classList       |          | list of strings | list of the class names, has to be in the same order as the output vector                                  | -             |
| name            | x        | string          | name of the classifier                                                                                     | -             |
| N               | x        | integer         | number of inputs of the classifier                                                                         | -             |
| M               | x        | integer         | number of outputs of the classifier                                                                        | -             |
| input_function  | x        | string          | function applied before the input ("average", "median_average" or "None")                                  | "average"     |
| output_function | x        | string          | function applied to the output ("logsoftmax", "sigmoid" or "None")                                         | "logsoftmax"  |
| is_multi_label  | x        | bool            | If true, threshold is applied to obtain multi-label annotations. Otherwise the most likely class is picked | false         |
| threshold       | x        | double          | threshold applied to the results of the output function                                                    | 0.3           |
| isInputAbsolute | x        | bool            | true if absolute vectors should be used, otherwise relative vectors are used as input                      | false         |
| mean            | x        | list of doubles | mean for each input, should be set if the classifier's training set was normalised                         | -             |
| variance        | x        | list of doubles | variance for each input, should be set if the classifier's training set was normalised                     | -             |
| preset_name     | x        | string          | name of the sensor's functionalisation preset                                                              | -             |


