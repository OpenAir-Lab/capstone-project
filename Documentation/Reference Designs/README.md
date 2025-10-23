# Reference Designs

Maintained in this documentation directory are the reference designs utilized in the "Schematic Recapture" process. 

## Schematic Recapture

Schematic recapture is the process of going from either provided vendor-specific design files created by their preferred Electronic Design Assistant (EDA) or provided generated fabrication outputs, i.e. GERBER and Excellon/NC Drill files.

The intent is to go from fabrication outputs or EDA-specific project files to a KiCAD project. 
- This means converting imported symbols into 'real' KiCAD symbols. If first-party symbols are not available or are incomplete, the next preference is for KiCAD built-in libraries.
- This also means converting imported footprints into 'real' KiCAD footprints. Many footprint libraries built-in to KiCAD lack 3D models and thus 3rd party services have been used to make the footprint information more complete.