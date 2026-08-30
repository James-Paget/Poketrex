PrintWriter output;
int vectrexHalfDimMax = 128;            // +-128 in each direction (essentially percentage of screen width using 8 bit signed values)
float vectrexAspectRatio = 0.75-0.15;   // 4:3 aspect ratio => 75% Y scalaing
float personalScaleFactor = 1.1;        // Additional scale factor to manual alter magnitude of vectors calculated by this factor (shrink or expand ouput result further)
float vectrexScaleFactor = 0.0;         // Overwritten on setup

PImage referenceImage = null;
PVector referenceDim = new PVector(0,0);
float referenceScaleRate = 0.01;    // Percentage change in each dim each frame a change occurs
int referenceRescale = 0;   // 0=Unaffected, 1 = SizeIncrease, -1=SizeDecrease

PVector origin = new PVector(0,0);

ArrayList<Float> vectorNodes = new ArrayList<Float>();          // Positions of nodes in format (y, x, intensity, ...) for intensity in (1,..,7) OR 0 for a skip

boolean toggleReference = true;

int backgroundColor = 0;

int verticalResolution = 40;//20;    // In pixels, jump between rows
int searchRadius = 5;           // Averages pixels value within this radius
float borderBoundary = 200.0;

// Setup
void setup() {
    size(800, 800);
    background(backgroundColor);

    vectrexScaleFactor = float(vectrexHalfDimMax)/float(width);

    try {
        referenceImage = loadImage("pokemon_starly_simple_sprite.png"); //loadImage("referenceImage.png");
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
        
        if(i>0) {
          pushStyle();
          stroke(0,255,0);
          line(vectorNodes.get(i-3), vectorNodes.get(i-3+1), vectorNodes.get(i), vectorNodes.get(i+1));
          popStyle();
        }
        
        stroke(255,0,0);
        fill(255,0,0);
        text(round(i/3), vectorNodes.get(i), vectorNodes.get(i+1)-10);
    }
}

void printVectorArrayOuputs() {
    /*
    . Print to console the arrays needed (in C format) for the vectrex lines() function to draw the image given
    . ** Note; The y coordinates are flipped due to processing using an XY axis of (RIGHT, UP) as (+, -), whereas the vectrex uses a more traditional (+, +)
    */
    int count = 0;
    output = createWriter("arrayOutputs.txt"); 
    /*
    . Writes the vectors to a file as flat_vectors set, which follows the structure:
        {spacing, originY, originX          <-- Only first 3 terms, not repeating
        x1, i1, x2, i2, ..., xn, in, snake_offset, 0, 
        x1, i1, x2, i2, ..., xm, im, snake_offset, 0, 
        ...}
        . spacing = Distance betwen adjacent lines/planes
        . originY/X = Distance from top left corner to first vector start point
        . 0 marks a new line is about to be started
        . snake_offset = Offset in X direction (y offset always is spacing)
        . x...= X offset for this vector (from current beam position, move by xn)
        . i...= Intensity of the line to be drawn
    . flat_vector_length = Number of terms in this flat vector full-list
    . Starts from top left corner
    . This method of storage reduces space taken up by this shaded image
    . Vectors stored so the beam 'snakes' around for speed improvement e.g.
            --->--\/
            \/-<----
            --->----
            ...
    . Intensity on scale 1-8 (NOT 0-7 to avoid 0 being used for situations other than line end) => 1==NO Beam Intensity, 8==MAX Beam Intensity
    */
    println("=== Output Start ===");
    output.println("const int8_t flat_vectors_array[...] = {");
    output.println("  "+ str(-int(personalScaleFactor*(convertToVectrexCoordinates(verticalResolution, true)))) +",");  // Spacing
    output.println("  "+ str( int(personalScaleFactor*(convertToVectrexCoordinates(origin.y, true)))) +",");  // OriginY
    output.println("  "+ str( int(personalScaleFactor*(convertToVectrexCoordinates(origin.x, false)))) +",");  // OriginX
    count += 3;
    for(int i=3; i<vectorNodes.size(); i+=3) {    // For each {y,x,i} vector grouping
      float pixelAverage = getPixelLineAverage(int(vectorNodes.get(i)), int(vectorNodes.get(i-3)), int(vectorNodes.get(i+1)), 3);
      //println(pixelAverage);
      int vectorIntensity = max(0, min(7, int(floor(7.0*(pixelAverage) /255.0))));
      if(round(vectorNodes.get(i+1)) != round(vectorNodes.get(i+1-3))) {
        output.println("  "+ str(int(convertToVectrexCoordinates( personalScaleFactor*(vectorNodes.get(i)-vectorNodes.get(i-3)) , false))) + ",0 , " );
      } else { 
        output.println("  "+ str(int(convertToVectrexCoordinates( personalScaleFactor*(vectorNodes.get(i)-vectorNodes.get(i-3)) , false))) + "," + str(int(vectorIntensity) +1) + " , " );  // Intensity +1 since flat_vector scale from 1-8 NOT 0-7, since 0 holds other significance in this system
      }
      count += 2;
    }
    output.println("};");
    output.println("flat_vector_length = "+str(count)+";");    

    output.flush(); // Writes the remaining data to the file
    output.close(); // Finishes the file
    println("=== Output Complete ===");
}


void calculateAutomaticHorizontalLinesMode() {
    /*
    . Calculates the points for the start and end of lines to form an image
    . The image is built out of horizontal lines

    . This program uses white pixels as intensity7, and black as intensity0 (no display)
      - changed from scale 1-8 when converted to a flat_vector
    */
    vectorNodes = new ArrayList<Float>();
    loadPixels();

    int count = 0;
    boolean atBorder = false;
    for(int j=0; j<height; j+=verticalResolution) {
        count = vectorNodes.size();
        for(int i=0; i<width; i++) {
            int insertIndex = -1;
            if(round(j/verticalResolution) % 2 == 1) { insertIndex = round(count/3); }  // On odd layers, add in reverse for snake-like effect
            float pixelAverage = getPixelAverage(i, j, searchRadius);
            
            if( (pixelAverage>borderBoundary) && !atBorder) {   // When you just hit the border, add a node
                // ### INTENSITY IRRELEVANT AT THIS STAGE ###
                addVectorNode(i, j, 0, insertIndex);
                atBorder = true;
            }
            if( (pixelAverage<borderBoundary) && atBorder) {
                // ### INTENSITY IRRELEVANT AT THIS STAGE ###
                addVectorNode(i, j, 0, insertIndex);
                atBorder = false;
            }
        }
    }
    
    println("== Nodes Calculated "+str(vectorNodes.size())+" ==");
}

void addVectorNode(float x, float y, int intensity, int index) {
    /*
    . A vectorNode is a set of 3 values consisting of the (y,x) position of the node AND an intensity
    . Intensity = 0 => skip (dont draw), and 0-7 for a true value
    */
    if(index!=-1) {
      vectorNodes.add(3*index, float(intensity));
      vectorNodes.add(3*index, y);
      vectorNodes.add(3*index, x);
    } else {  //-1 => do at end
      vectorNodes.add(x);
      vectorNodes.add(y);
      vectorNodes.add(float(intensity));

    }
    // Hence will be listed as (x,y,i, ...)
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
    int count = 0;
    float lineValue = 0.0;
    for(int j=max(0, p_y-dy_radius); j<min(height, p_y+dy_radius); j++) {
        for(int i=min(p1_x, p2_x); i<max(p1_x, p2_x); i++) {
            lineValue += (red(pixels[j*width +i]) + green(pixels[j*width +i]) + blue(pixels[j*width +i]))/3;
            count++;
        }
    }
    return (lineValue/count);
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
