import processing.serial.*; //<>// //<>//
import controlP5.*;
import java.util.Map;


Serial     serialPort;
String[]   availablePorts;
ControlP5  cp5;
CheckBox   checkbox;
Button     uploadButton;
PFont      font;
color      labelColor;
int        startTime;
boolean    connected = false;

void setup() {
  size( 800, 520 );
  background( 0 );
  smooth();

  printArray( Serial.list() );
  availablePorts = Serial.list();
  for ( int i = 0; i < availablePorts.length; i++) {
    if ( availablePorts[ i ].indexOf( "RCswitch" ) > 0 || i == availablePorts.length -1 ) {
      println(" use serial port nr: " + i ); 
      serialPort = new Serial( this, availablePorts[ i ], 9600 );
      break;
    }
  }

  for ( int i = 0; i< 10 && !connected; i++) {
    println("connect...");
    serialPort.write("C");
    delay(200);
    if ( serialPort.available() > 0 && serialPort.readChar() == 'C') {
      connected = true;
      serialPort.clear();
      println("connected");
    }
  }

  startTime = millis();

  font = loadFont( "OCRAbyBT-Regular-18.vlw" );
  labelColor = color( 178 );

  cp5 = new ControlP5(this);
  checkbox = cp5.addCheckBox("checkBox");
  addCheckBoxes( cp5, font, labelColor, checkbox );

  // upload button
  PImage[] uploadImgs = { 
    loadImage( "upload1.png" ), loadImage( "upload2.png" ), loadImage( "upload3.png" )
  };
  uploadButton = new Button( cp5, "upload" );
  uploadButton.setValue( 1 );
  uploadButton.setPosition( 366, 440 );
  uploadButton.setImages( uploadImgs );
  uploadButton.updateSize();
}



void draw() {
  if ( serialPort.available() > 0 && connected) {  
    println( "read: " + serialPort.readString() );
  }
}

public void upload( int btValue ) {
  byte lowByte = 0;
  byte highByte  = 0;

  if ( millis()-startTime < 1000 ) {
    return;
  } 
  float[] values = checkbox.getArrayValue();
  println( "upload" );
  printArray( checkbox.getArrayValue() );

  for ( int i = 0; i < 10; i++ ) {
    int rowValue = 0;
    for ( int j = 0; j < 16; j++ ) {
      if ( int(values[ i*16 + j ] ) == 1  ) {
        rowValue = rowValue | (1 << j);
        println( "set: " + i + ", " + j + " [" + ( i*10 + j  ) +"]");
      }
    }
    println( "row: " + binary( rowValue ) );

    lowByte =  byte( rowValue & 0x00FF );
    highByte = byte( ( rowValue & 0xFF00) >> 8 );

    serialPort.write( lowByte );
    serialPort.write( highByte );

    //    println( binary( highByte ) + " " + binary( lowByte ) );
  }
}

void addCheckBoxes( ControlP5 control, PFont labelFont, color labelColor, CheckBox stats ) {
  Textlabel textlabelOutput;
  Textlabel[] textlabelSwich;

  textlabelSwich = new Textlabel[ 16 ];

  textlabelOutput = control.addTextlabel( "textlabelOutput" );
  textlabelOutput.setText( "output 01 - 16 ");
  textlabelOutput.setPosition( 195, 20 );
  textlabelOutput.setColorValue( labelColor );
  textlabelOutput.setFont( labelFont );


  stats.setPosition( 120, 50 );  
  stats.hideLabels();
  stats.hideBar();
  stats.setColorLabel( color( 0 ) );
  stats.setSize( 20, 20 );
  stats.setItemsPerRow( 16 );
  stats.setSpacingColumn( 15 );
  stats.setSpacingRow( 15 );

  for ( int i=0; i<10; i++ ) {

    textlabelSwich[ i ] = control.addTextlabel( "inLabel" + i );
    textlabelSwich[ i ].setText( "pulse " + nf( (i + 1), 2 ) );
    textlabelSwich[ i ].setPosition( 20, 55 + i*35 );
    textlabelSwich[ i ].setColorValue( labelColor );
    textlabelSwich[ i ].setFont( labelFont );

    for ( int j=0; j<16; j++ ) {    
      stats.addItem( j + "c" + i, i+(j*10) );
    }
  }
}


// here we overwrite PApplet's main entry point (for application mode)
// we're parsing all commandline arguments and copy only the relevant ones
/*
static public void main(String args[]) {
 String[] newArgs=new String[args.length+1];
/*********************************************************
/* IMPORTANT: replace this with the name of your sketch  *
/*********************************************************/
/*  newArgs[0]="RC_Switch_CFG";
 
 // pass on to PApplet entry point
 PApplet.main(newArgs);
 }
 */
