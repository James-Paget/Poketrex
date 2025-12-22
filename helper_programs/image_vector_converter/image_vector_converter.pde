/*
. Program written to allow;
    (A) Manual creation of a vector list (e.g. Place ordered points by hand to provide a vector lines usable 
        by the vectrex; an image can be overlayed behind to essentially 'trace' it)
    (B) Automatic creation of a vector list from an image provided
. The generated vector list will be a set of relative vectors applicable to the 'lines(nlines, list)' function 
    declared in vectrex/bios.h, as provided by CMOC
. Another list will also be provided which specifies the indices in the vector list where the line should not be 
    drawn, just the beam moved (so the image does not have to completely connected)
*/

String creationMode = "NULL";
int vectrexHalfDimMax = 128;        // +-128 in each direction (essentially percentage of screen width using 8 bit signed values)
float personalScaleFactor = 1.0;    // Additional scale factor to manual alter magnitude of vectors calculated by this factor (shrink or expand ouput result further)

PImage referenceImage = null;
PVector referenceDim = new PVector(0,0);
float referenceScaleRate = 0.01;    // Percentage change in each dim each frame a change occurs
int referenceRescale = 0;   // 0=Unaffected, 1 = SizeIncrease, -1=SizeDecrease

Boolean toggleControls = true;
Boolean toggleVectorNodes = true;
Boolean toggleVectors = true;
Boolean toggleReference = true;
Boolean toggleOrigin = true;
PVector origin = new PVector(0,0);
ArrayList<PVector> vectorNodes = new ArrayList<PVector>();      // Positions of nodes
ArrayList<Integer> skippedNodes = new ArrayList<Integer>();     // Indices in vectorNodes of skipped nodes


// Setup
void setup() {
    creationMode = "MANUAL";
    size(800, 800);
    background(255, 255, 255);

    try {
        referenceImage = loadImage("referenceImage.png");
        referenceDim = new PVector(referenceImage.width, referenceImage.height);
    } catch(Exception e) {
        println("Invalid reference image; expecting file of format 'referenceImage.png' in /data folder");
    }
}

void draw() {
    background(255, 255, 255);
    displayGeneralMode();
    switch(creationMode) {
        case "MANUAL":
            displayManualMode();
            break;
        case "AUTO":
            displayAutoMode();
            break;
    }
}

void keyPressed() {
    switch(creationMode) {
        case "MANUAL":
            if(key == '1') {    // Place origin
                origin = new PVector(mouseX, mouseY);
                println("* Origin placed");
            }
            if(key == '2') {    // Place vectorNode
                vectorNodes.add( new PVector(mouseX, mouseY) );
                println("* VectorNode placed");
            }
            if(key == '3') {    // Remove last placed  vectorNode
                if(vectorNodes.size() > 0) { vectorNodes.remove( vectorNodes.size()-1 );println("* VectorNode removed"); }
            }
            if(key == '4') {    // Mark latest node as a skipped node
                skippedNodes.add( vectorNodes.size()-1 );
                println("* Skipped node set");
            }
            if(key == '5') {    // Remove latest marked skipped node
                if(skippedNodes.size() > 0) { skippedNodes.remove( skippedNodes.size()-1 );println("* Skipped node removed"); }
            }
            // Toggles
            if(key == '6') { toggleControls = !toggleControls; }
            if(key == '7') { toggleVectorNodes = !toggleVectorNodes; }
            if(key == '8') { toggleVectors = !toggleVectors; }
            if(key == '9') { toggleReference = !toggleReference; }
            if(key == '0') { toggleOrigin = !toggleOrigin; }
            break;
        case "AUTO":
            //pass
            break;
    }
    if(key == 'a') {
        printVectorArrayOuputs();
    }
    if(key == 'w') { referenceRescale = 1; }
    if(key == 's') { referenceRescale = -1; }
    if(key == 'q') { personalScaleFactor -= 0.05; }
    if(key == 'e') { personalScaleFactor += 0.05; }
}
void keyReleased() {
    if(key == 'w') { referenceRescale = 0; }
    if(key == 's') { referenceRescale = 0; }
}


// General methods
void displayGeneralMode() {
    /*
    . Displays all relevant to all modes for the program
    . This includes;
        . Background reference image
        . Manually/Automatically chosen origin position (important for vectrex display only)
        . Manually placed vector nodes + skip nodes
        . The resulting output vectors
        . Hotkeys to toggle each of these layers
    */
    if(toggleReference) {displayReferenceImage();}
    if(toggleOrigin) {displayOrigin();}
    if(toggleVectorNodes) {displayVectorNodes(true);}
    if(toggleVectorNodes) {displaySkippedNodes();}
    if(toggleVectors) {displayVectorOutput();}
    if(toggleControls) {displayControls();}
}
void displayReferenceImage() {
    /*
    . Displays the loaded reference if image if possible
    . Displays nothing if not loaded
    */
    if(referenceImage!=null) {
        referenceDim.x += referenceDim.x*referenceScaleRate*referenceRescale;
        referenceDim.y += referenceDim.y*referenceScaleRate*referenceRescale;
        image(referenceImage, 0, 0, referenceDim.x, referenceDim.y);
    }
}
void displayOrigin() {
    pushStyle();
    fill(0,100,0);
    noStroke();
    ellipse(origin.x, origin.y, 10, 10);
    popStyle();
}
void displayVectorNodes(Boolean includeOrder) {
    /*
    . Displays the nodes used to connect the vectors to produce the image
    . Vector nodes will connect to subsequent vector nodes to form vectors, but will NOT form a vector if the following node is a 'skipped node' (marked in a separate list)
    */
    for(int i=0; i<vectorNodes.size(); i++) {
        pushStyle();
        fill(0,0,0);
        noStroke();
        ellipse(vectorNodes.get(i).x, vectorNodes.get(i).y, 10, 10);

        if(includeOrder) {
            text(i, vectorNodes.get(i).x, vectorNodes.get(i).y-10);
        }

        popStyle();
    }
}
void displaySkippedNodes() {
    /*
    . Displays the nodes used to connect the vectors to produce the image
    . Vector nodes will connect to subsequent vector nodes to form vectors, but will NOT form a vector if the following node is a 'skipped node' (marked in a separate list)
    */
    for(int i=0; i<skippedNodes.size(); i++) {
        if(skippedNodes.get(i) < vectorNodes.size()) {
            pushStyle();
            fill(200,0,0);
            noStroke();
            ellipse(vectorNodes.get( skippedNodes.get(i) ).x, vectorNodes.get( skippedNodes.get(i) ).y, 15, 15);
            popStyle();
        }
    }
}
void displayVectorOutput() {
    /*
    . Displays the actual vectors connecting each node according to the parameters given
    . This should reflect the output
    */
    if(vectorNodes.size() > 1) {    // If at least 1 vector can be formed
        pushStyle();
        noFill();
        stroke(0,0,50);
        strokeWeight(2);
        for(int i=1; i<vectorNodes.size(); i++) {
            Boolean isSkippedNode = false;
            for(int j=0; j<skippedNodes.size(); j++) {
                if(skippedNodes.get(j) == i) {
                    isSkippedNode = true;
                    break;
                }
            }
            if(!isSkippedNode) {    // If not a skipped node, connect to the prior node
                line(
                    vectorNodes.get(i-1).x, vectorNodes.get(i-1).y, 
                    vectorNodes.get(i  ).x, vectorNodes.get(i  ).y
                );
            }
        }
        popStyle();
    }
}
void displayControls() {
    /*
    . Displays the general hotkeys for the program
    */
    float textSize = 15;
    pushStyle();
    fill(0,0,0);
    textSize(textSize);
    text("1 = Place origin", 0, 1*textSize);
    text("2 = Place  vectorNode", 0, 2*textSize);
    text("3 = Remove vectorNode", 0, 3*textSize);
    text("4 = Place  skippedNode", 0, 4*textSize);
    text("5 = Remove skippedNode", 0, 5*textSize);
    text("6 = Toggle controls", 0, 6*textSize);
    text("7 = Toggle vectorNodes", 0, 7*textSize);
    text("8 = Toggle vectors", 0, 8*textSize);
    text("9 = Toggle referenceImage", 0, 9*textSize);
    text("0 = Toggle origin", 0, 10*textSize);
    text("---", 0, 11*textSize);
    text("a = Get vector array output", 0, 12*textSize);
    text("w = Increase reference size", 0, 13*textSize);
    text("s = Decrease reference size", 0, 14*textSize);
    text("q = Decrease personalScaleFactor", 0, 15*textSize);
    text("e = Increase personalScaleFactor", 0, 16*textSize);
    text("personalScaleFactor = "+str(personalScaleFactor), 0, 17*textSize);
    popStyle();
}

void printVectorArrayOuputs() {
    /*
    . Print to console the arrays needed (in C format) for the vectrex lines() function to draw the image given
    . ** Note; The y coordinates are flipped due to processing using an XY axis of (RIGHT, UP) as (+, -), whereas the vectrex uses a more traditional (+, +)
    */
    float vectrexScaleFactor = 2.0*vectrexHalfDimMax/width;   // Convert from processing pixel based units to vectrex coordinates
    // + Use personalScaleFactor

    println("=== ORIGIN/MOVE START ===");
    if(vectorNodes.size() > 0) { println("move("+str(int( (vectorNodes.get(0).x-origin.x)*vectrexScaleFactor*personalScaleFactor ))+","+str(-int( (vectorNodes.get(0).y-origin.y)*vectrexScaleFactor*personalScaleFactor ))+");"); }
    println("=== ORIGIN/MOVE END ===");
    
    println("=== VECTOR NODES START ===");
    println("const int8_t vectors_array["+str(int(2*vectorNodes.size()))+"] = {");
    if(vectorNodes.size() > 1) {    // If you have at least 1 vector (e.g. 2 points -> doesn't account for skips at this minimum; *Be Careful*)
        for(int i=1; i<vectorNodes.size(); i++) {
            println( "  "+str(int( (vectorNodes.get(i).x-vectorNodes.get(i-1).x)*vectrexScaleFactor*personalScaleFactor ))+", "+str(-int( (vectorNodes.get(i).y-vectorNodes.get(i-1).y)*vectrexScaleFactor*personalScaleFactor ))+", " );
        }
    }
    println("};");
    println("=== VECTOR NODES END ===");

    println("=== SKIPPED NODES START ===");
    println("const uint8_t skipped_array["+str(int(skippedNodes.size()))+"] = {");
    for(int j=0; j<skippedNodes.size(); j++) {
        println( "  "+str(int(skippedNodes.get(j)))+", " );
    }
    println("};");
    println("=== SKIPPED NODES END ===");
}


// Manual specific methods
void displayManualMode() {
    /*
    . Displays all relevant to the manual mode for the program
    . This includes;
        ...
    */
    //pass
}


// Automatic specific methods
void displayAutoMode() {
    /*
    . Displays all relevant to the auto mode for the program
    . This includes;
        ...
    */
    //pass
}
