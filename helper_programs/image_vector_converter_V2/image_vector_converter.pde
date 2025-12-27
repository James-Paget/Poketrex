PrintWriter output;
int vectrexHalfDimMax = 128;            // +-128 in each direction (essentially percentage of screen width using 8 bit signed values)
float vectrexAspectRatio = 0.75-0.15;   // 4:3 aspect ratio => 75% Y scalaing
float personalScaleFactor = 1.0;        // Additional scale factor to manual alter magnitude of vectors calculated by this factor (shrink or expand ouput result further)
float vectrexScaleFactor = 0.0;         // Overwritten on setup

PImage referenceImage = null;
PVector referenceDim = new PVector(0,0);
float referenceScaleRate = 0.01;    // Percentage change in each dim each frame a change occurs
int referenceRescale = 0;   // 0=Unaffected, 1 = SizeIncrease, -1=SizeDecrease

PVector origin = new PVector(0,0);

ArrayList<Float> vectorNodes = new ArrayList<Float>();          // Positions of nodes in format (y, x, intensity, ...) for intensity in (1,..,7) OR 0 for a skip
ArrayList<Integer> vectorArray = new ArrayList<Integer>();      // Vectors for the image, as would be read by the vectrex

boolean toggleReference = true;

int backgroundColor = 0;

int verticalResolution = 20;    // In pixels, jump between rows
int searchRadius = 5;           // Averages pixels value within this radius
float borderBoundary = 200.0;

// Setup
void setup() {
    size(800, 800);
    background(backgroundColor);

    vectrexScaleFactor = float(vectrexHalfDimMax)/float(width);

    try {
        referenceImage = loadImage("referenceImage.png");
        referenceDim = new PVector(referenceImage.width, referenceImage.height);
    } catch(Exception e) {
        println("Invalid reference image; expecting file of format 'referenceImage.png' in /data folder");
    }
}

void draw() {
    background(backgroundColor);

    if(toggleReference) { displayReferenceImage(); }
    displayOrigin();
    displayNodeOutput();
    displayVectorOutput();
    // displayControls();
}

void keyPressed() {
    if(key == 'a') { printVectorArrayOuputs(); }
    if(key == 'b') { calculateAutomaticHorizontalLinesMode(); }

    if(key == 'w') { referenceRescale = 1; }
    if(key == 's') { referenceRescale = -1; }

    if(key == '1') {
        if(backgroundColor != 0) {
            backgroundColor = 0;
        } else {
            backgroundColor = 255;
        }
    }
    if(key == '2') {toggleReference = !toggleReference;}
    if(key == '3') {
        origin = new PVector(mouseX, mouseY);
        println("== Origin Set To Mouse ==");
    }
}
void keyReleased() {
    if(key == 'w') { referenceRescale = 0; }
    if(key == 's') { referenceRescale = 0; }
}


// General methods
void displayReferenceImage() {
    /*
    . Displays the loaded reference if image if possible
    . Displays nothing if not loaded
    */
    if(referenceImage!=null) {
        referenceDim.x += referenceDim.x*referenceScaleRate*referenceRescale;
        referenceDim.y += referenceDim.y*referenceScaleRate*referenceRescale;
        image(referenceImage, 0, 0, referenceDim.x, referenceDim.y);
        filter(GRAY);
    }
}
void displayOrigin() {
    pushStyle();
    fill(0,100,0);
    noStroke();
    ellipse(origin.x, origin.y, 10, 10);
    popStyle();
}

void displayNodeOutput() {
    for(int i=0; i<vectorNodes.size(); i+=3) {
        // float intensity = (255.0*vectorArray.get(i+2))/(7.0);
        pushStyle();
        noStroke();
        // fill(intensity);
        fill(255, 0, 0);
        ellipse(vectorNodes.get(i), vectorNodes.get(i+1), 10, 10);
        popStyle();
    }
}

void displayVectorOutput() {
    /*
    . Displays the actual vectors connecting each node according to the parameters given
    . This should reflect the output
    */
    PVector beamPosition = new PVector(origin.x, origin.y);
    for(int i=0; i<vectorArray.size(); i+=3) {
        if(vectorArray.get(i+2) > 0) {
            float intensity = (255.0*vectorArray.get(i+2))/(7.0);
            pushStyle();
            noFill();
            strokeWeight(max(5,vectorArray.get(i+2)));
            stroke(intensity);
            line(
                beamPosition.x,
                beamPosition.y,
                beamPosition.x +vectorArray.get(i+1),
                beamPosition.y +vectorArray.get(i)
            );
            popStyle();
        }
        beamPosition.x += vectorArray.get(i+1);
        beamPosition.y += vectorArray.get(i);
    }
}

void printVectorArrayOuputs() {
    /*
    . Print to console the arrays needed (in C format) for the vectrex lines() function to draw the image given
    . ** Note; The y coordinates are flipped due to processing using an XY axis of (RIGHT, UP) as (+, -), whereas the vectrex uses a more traditional (+, +)
    */
    output = createWriter("arrayOutputs.txt"); 

    output.println("=== ORIGIN/MOVE START ===");
    output.println("int8_t origin[2] = {"+str(int(convertToVectrexCoordinates(origin.y, true)))+", "+str(int(convertToVectrexCoordinates(origin.x, false)))+"};");
    output.println("=== ORIGIN/MOVE END ===");

    output.println("=== VECTOR ARRAY START ===");
    output.println("int8_t vector_array["+str(int(vectorArray.size()))+"] = {");
    for(int i=0; i<vectorArray.size(); i+=3) {
        output.println("  "+str(int(convertToVectrexCoordinates(-vectorArray.get(i), true)))+", "+str(int(convertToVectrexCoordinates(vectorArray.get(i+1), false)))+", "+str(int(vectorArray.get(i+2)))+", ");
    }
    output.println("};");
    output.println("=== VECTOR ARRAY END ===");

    output.flush(); // Writes the remaining data to the file
    output.close(); // Finishes the file
    println("=== Output Complete ===");
}


void calculateAutomaticHorizontalLinesMode() {
    /*
    . Calculates the points for the start and end of lines to form an image
    . The image is built out of horizontal lines

    . This program uses white pixels as intensity7, and black as intensity0 (no display)
    */
    vectorNodes = new ArrayList<Float>();
    loadPixels();

    boolean atBorder = false;
    for(int j=0; j<height; j+=verticalResolution) {
        for(int i=0; i<width; i++) {
            float pixelAverage = getPixelAverage(i, j, searchRadius);
            if( (pixelAverage>borderBoundary) && !atBorder) {   // When you just hit the border, add a node
                // ### INTENSITY IRRELEVANT AT THIS STAGE ###
                addVectorNode(i, j, 0);
                atBorder = true;
            }
            if( (pixelAverage<borderBoundary) && atBorder) {
                // ### INTENSITY IRRELEVANT AT THIS STAGE ###
                addVectorNode(i, j, 0);
                atBorder = false;
            }
        }
    }
    calculateVectorArrayFromNodes();
    println("== Nodes Calculated "+str(vectorNodes.size())+" ==");
    println("== Lines Calculated "+str(vectorArray.size())+" ==");
}

void calculateVectorArrayFromNodes() {
    // ###
    // ### COULD IMPROVE PERFORMANCE BY SNAKING THE VECTORS, SO DOESNT HAVE TO MOVE ALL THE WAY BACK TO THE LEFT
    // ###
    vectorArray = new ArrayList<Integer>();

    if(vectorNodes.size() > 3) {
        // Add origin vector (invisible/skipped)
        addVectorArray(
            int(floor(vectorNodes.get(0) -origin.x)), 
            int(floor(vectorNodes.get(1) -origin.y)),
            0
        );
        // Add line vectors
        for(int i=3; i<vectorNodes.size(); i+=3) {  // (y, x, intensity, ...)
            if(int(floor(vectorNodes.get(i-2))) == int(floor(vectorNodes.get(i+1)))) {    // If the two nodes are the same height, draw correctly intense line between them
                float pixelAverage = getPixelLineAverage(int(vectorNodes.get(i-3)), int(vectorNodes.get(i)), int(vectorNodes.get(i+1)), 3);
                int vectorIntensity = max(0, min(7, int(floor(7.0*(pixelAverage) /255.0))));
                //max(0, min(7, int(floor( (vectorNodes.get(i-1) +vectorNodes.get(i+2))/2.0 ))));
                addVectorArray(
                    int(floor(vectorNodes.get(i) -vectorNodes.get(i-3))), 
                    int(floor(vectorNodes.get(i+1) -vectorNodes.get(i-2))), 
                    vectorIntensity
                );
            } else {    // If they are a different height, the vector joining them should be invisible/skipped
                addVectorArray(
                    int(floor(vectorNodes.get(i) -vectorNodes.get(i-3))), 
                    int(floor(vectorNodes.get(i+1) -vectorNodes.get(i-2))), 
                    0
                );
            }
        }
    }
}

void addVectorNode(float x, float y, int intensity) {
    /*
    . A vectorNode is a set of 3 values consisting of the (y,x) position of the node AND an intensity
    . Intensity = 0 => skip (dont draw), and 0-7 for a true value
    */
    vectorNodes.add(x);
    vectorNodes.add(y);
    vectorNodes.add(float(intensity));
}
void addVectorArray(int dx, int dy, int intensity) {
    vectorArray.add(dy);
    vectorArray.add(dx);
    vectorArray.add(intensity);
}

float getPixelAverage(int pixelOriginX, int pixelOriginY, int searchRadius) {
    // Assumes the pixels have been loaded
    float pixelValue = 0.0;
    for(int j=max(0, pixelOriginY-searchRadius); j<min(height, pixelOriginY+searchRadius); j++) {
        for(int i=max(0, pixelOriginX-searchRadius); i<min(width, pixelOriginX+searchRadius); i++) {
            pixelValue += sqrt( pow(red(pixels[j*width +i]), 2) +pow(green(pixels[j*width +i]), 2) +pow(blue(pixels[j*width +i]), 2) );
        }
    }
    pixelValue /= ( min(width, pixelOriginX+searchRadius)-max(0, pixelOriginX-searchRadius) )*( min(height, pixelOriginY+searchRadius)-max(0, pixelOriginY-searchRadius) );
    return pixelValue;
}
float getPixelLineAverage(int p1_x, int p2_x, int p_y, int dy_radius) {
    float lineValue = 0.0;
    for(int j=max(0, p_y-dy_radius); j<min(height, p_y+dy_radius); j++) {
        for(int i=min(p1_x, p2_x); i<max(p1_x, p2_x); i++) {
            lineValue += sqrt( pow(red(pixels[j*width +i]), 2) +pow(green(pixels[j*width +i]), 2) +pow(blue(pixels[j*width +i]), 2) );
        }
    }
    lineValue /= ( max(p1_x, p2_x)-min(p1_x, p2_x) )*( min(height, p_y+dy_radius)-max(0, p_y-dy_radius) );
    return lineValue;
}

int convertToVectrexCoordinates(float processingUnit, Boolean isY) {
    /*
    . Converts an absolute or relative coordinate from processing pixel coordinates to the vectrex's scale

    ** NOTE; If this function uses purely floor() or ceil() you will get noticable drift when displaying on the vectrex's very discrete display => round() works best
    */
    float aspectRatioFix = 1.0;
    if(isY) {aspectRatioFix = vectrexAspectRatio;}
    return int(round(processingUnit*vectrexScaleFactor*personalScaleFactor*aspectRatioFix));
}