//WirelessFilPilote1 V2.0

_part = "both"; // [box, lid, both]

//$fn=30;
$fn=60;

//PCB dimensions
_roundCorner=6.985;
_cornerSpacingX=20.32;
_cornerSpacingY=20.32;
_loose=1;
_pcbX = _cornerSpacingY+2*_roundCorner;
_pcbY = _cornerSpacingX+2*_roundCorner;

_innerDimensions = true;

// width of the box (affects the length of the hinges)
_width = _pcbX+_loose; // [10:300]

// length of the box
_length = _pcbY+_loose; // [10:300]

// height of the box
_height = 20; // [4:300]

// rounding radius of the corners of the box
_rounding = _roundCorner+1.5; // [1:50] //_roundCorner+_sidewallThickness

// thickness of the walls around the magnets
_minimumWallThickness = 0.5;

// thickness of the side walls without the hinge
_sidewallThickness = 1.5;

// thickness of the bottom of the box or the top of the lid
_horizontalThickness = 1.25;

// diameter of magnets in the hinge
_magnetDiameter = 2.5;

// height magnets in the hinge
_magnetHeight = 1;

// gap bewteen the lid and box parts
_hingeGap = 0.4;

// Size of lid's spheres that go into box holes
_hingeOverlap = 0.15;

_lidRadius = 0;

module roundRect(width, depth, height, round) {
	round = min(round, width/2);
	linear_extrude(height=height,center=true) offset(r=round) square([width-2*round,depth-2*round],center=true);
}

module roundRectFillet(width, depth, height, round, radius) {
    
    radius=max(0.00001,radius);
    
    minkowski(){
        union(){
            
            translate([0,0,(height-radius)/2])cylinder(r=radius,h=height-radius,center=true);
                
            translate([0,0,height-(radius>height?height:radius)])
            resize([0,0,radius>height?height:0])
            difference(){
                sphere(r=radius, center=true);
                translate([0,0,-radius/2]) cube([2*radius,2*radius,radius],center=true);
            }
        }
    
        roundRect(width-(2*radius),depth-(2*radius),0.000001,round);
    }
}

module makeBase(width, depth, height, rounding, minimum, side, bottom, mdiameter, mheight, gap) {
	eps = 0.1;
	iwidth = width - side*2;
	idepth = depth - mdiameter*2 - minimum*4;
	hingeCutZ = mdiameter + minimum*2/* + gap*/;
	hingeCutWidth = width - rounding*2 - mheight*2;
	fillet = mdiameter/2 + minimum;

    union(){
        difference() {
            translate([0,0,height/2]) 
            difference() {
                roundRect(width, depth, height, rounding);
                translate([0,0,bottom]) {
                    roundRect(iwidth, idepth, height, rounding-side);
                    //cube(size=[iwidth, idepth, height],center=true);
                }
            }
            
    
            // hinge cutout
            translate([0,0,height - hingeCutZ/2 + eps/2]) {
                cube(size=[hingeCutWidth, depth+eps, hingeCutZ + eps], center=true);
            }
            
            // sphere cutout
            for (x=[-1,1])
            for (y=[-1,1]) {
                translate([x * (hingeCutWidth/2 - eps/2), y*(depth/2 - minimum - mdiameter/2), height - minimum - mdiameter/2])
                sphere(d=mdiameter);
            }

            //WirelessFilPilote1 bornier Cutout

            //bornier3
            translate([-width/2-0.1,1+4/2+0.635/2,bottom+5/*5mm from bottom*/])cube([(width-iwidth)/2+0.2,4,6]);
            translate([-width/2-0.1,-4/2+0.635/2,bottom+5/*5mm from bottom*/])cube([(width-iwidth)/2+0.2,4,6]);     
            translate([-width/2-0.1,-5-4/2+0.635/2,bottom+5/*5mm from bottom*/])cube([(width-iwidth)/2+0.2,4,6]);

            //WirelessFilPilote1 Texts

            //bornier3
            translate([-width/2+0.5,-9.5,12.5])rotate([90,0,-90])linear_extrude(0.6)text(text="N",font="Liberation Sans:style=Bold",size=5,halign="center");
            translate([-width/2+0.5,0.635/2,13.5])rotate([90,0,-90])linear_extrude(0.6)text(text="P",font="Liberation Sans:style=Bold",size=5,halign="center");
            translate([-width/2+0.5,10,12.5])rotate([90,0,-90])linear_extrude(0.6)text(text="FP",font="Liberation Sans:style=Bold",size=5,halign="center");

        }

        //WirelessFilPilote1 additions

    }
}


module makeLid(width, depth, rounding, minimum, side, bottom, mdiameter, mheight, gap, overlap) {
	eps = 0.1;
	hingeWidth = width - rounding*2 - mheight*2 - gap*2;
	hingeSize = mdiameter + minimum*2;
    
    //WirelessFilPilote1
    idepth = depth - mdiameter*2 - minimum*4;
	
	difference() {
		union() {
			translate([0,0,bottom/2]) {
				roundRect(width, depth, bottom, rounding);
			}	

			// hinges
			for (s=[-1,1]) {
				translate([0,s*(depth/2 - hingeSize/2),bottom + hingeSize/2]) {
					hull() {
						rotate([0,90,0]) {
							cylinder(r=hingeSize/2, h=hingeWidth,center=true);
						}
						translate([0,s*-hingeSize/4,0]) {
							cube(size=[hingeWidth, hingeSize/2, hingeSize], center=true);
						}
						translate([0,0,-hingeSize/4-eps]) {
							cube(size=[hingeWidth, hingeSize, hingeSize/2], center=true);
						}
					}
				}
			}
            // sphere additions
            for (x=[-1,1])
            for (y=[-1,1]) {
                translate([x * (hingeWidth/2 - eps), y*(depth/2 - minimum - mdiameter/2), mdiameter/2 + bottom + minimum]){
                    difference(){
                        sphere(d=mdiameter);
                        translate([x*(mdiameter/2+eps+gap+overlap),0,0]) cube([mdiameter,mdiameter,mdiameter],center=true);
                    }
                }
            }
            
            //WirelessFilPilote1 Additions
		}


        //WirelessFilPilote1 cutouts

        //bornier3
        hull(){
            translate([13.335,5+0.635/2,0])cylinder(d=4.5,h=bottom);
            translate([13.335,+0.635/2,0])cylinder(d=4.5,h=bottom);
            translate([13.335,-5+0.635/2,0])cylinder(d=4.5,h=bottom);
        }

        //WirelessFilPilote1 Texts
        //bornier3
        translate([8,-10.5,0.5])rotate([0,-180,-90])difference(){
            linear_extrude(0.6)offset(r=0.7)text(text="N",font="Liberation Sans:style=Bold",size=3.5,halign="center");
            linear_extrude(0.6)text(text="N",font="Liberation Sans:style=Bold",size=3.5,halign="center");
        }
        translate([6,0.635/2,0.5])rotate([0,-180,-90])difference(){
            linear_extrude(0.6)offset(r=0.7)text(text="P",font="Liberation Sans:style=Bold",size=3.5,halign="center");
            linear_extrude(0.6)text(text="P",font="Liberation Sans:style=Bold",size=3.5,halign="center");
        }
        translate([8,12.5,0.5])rotate([0,-180,-90])difference(){
            linear_extrude(0.6)offset(r=0.7)text(text="FP",font="Liberation Sans:style=Bold",size=3.5,halign="center");
            linear_extrude(0.6)text(text="FP",font="Liberation Sans:style=Bold",size=3.5,halign="center");
        }

        //Brand :-)
        translate([0,0,0.5])rotate([0,-180,-90])difference(){
            linear_extrude(0.6)offset(r=0.7)text(text="WFP1",font="Liberation Sans:style=Bold",size=4,halign="center");
            linear_extrude(0.6)text(text="WFP1",font="Liberation Sans:style=Bold",size=4,halign="center");
        }
        //translate([4.5,-9.5,0.5])rotate([0,-180,-180])linear_extrude(0.6)text(text="WirelessFilPilote1",font="Liberation Sans:style=Bold",size=2.4,halign="center");
	}

}

module make() {
	// minimal error checking
	eps = 0.1;
    
	rounding = max(_rounding, _sidewallThickness + eps);
	height = max(_height, _horizontalThickness + _magnetDiameter + _minimumWallThickness*2 + _hingeGap);


    //if innerDimensions then recalculate
    if(_innerDimensions == true){
        
        width=_width+2*_sidewallThickness;
        length=_length+2*(_magnetDiameter+2*_minimumWallThickness);
        height=height+_horizontalThickness;
        
        if (_part == "box" || _part == "both") {
            makeBase(width, length, height, rounding, 
                        _minimumWallThickness, _sidewallThickness, _horizontalThickness, 
                        _magnetDiameter, _magnetHeight, _hingeGap);
        }
        if (_part == "lid" || _part == "both") {
            translate([0,0,height+_horizontalThickness+0.1]) rotate([0,180,0]) makeLid(width, length, rounding, 
                        _minimumWallThickness, _sidewallThickness, _horizontalThickness, 
                        _magnetDiameter, _magnetHeight, _hingeGap, _hingeOverlap);
        }
    }
    else{
        if (_part == "box" || _part == "both") {
            makeBase(_width, _length, height, rounding, 
                        _minimumWallThickness, _sidewallThickness, _horizontalThickness, 
                        _magnetDiameter, _magnetHeight, _hingeGap);
        }
        if (_part == "lid" || _part == "both") {
            translate([0,0,height+_horizontalThickness+0.1]) rotate([0,180,0]) makeLid(_width, _length, rounding, 
                        _minimumWallThickness, _sidewallThickness, _horizontalThickness, 
                        _magnetDiameter, _magnetHeight, _hingeGap, _hingeOverlap);
        }

		
	}
}

make();

