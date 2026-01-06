$fa = 1;
$fs = 0.4;

pcb_width = 81.5;
pcb_height = 53.5;
pcb_thickness = 2;
crap_thickness = 16;

floor_thickness = 1.5;
wall_thickness = 1.6;

screen_top_height = 15;
screen_thickness = 10.25;

viewport_height = 37;
viewport_width = 30;
viewport_to_screen_top = 9;
battery_padding = 9;

screw_insert_radius = 2.2;
screw_hole_radius = 1.5;
screw_insert_height = 6.5;
screw_holder_radius = screw_insert_radius+1.5;
screw_holder_height = 8;

case_width = pcb_width;
case_height = pcb_height + screen_top_height + battery_padding;
case_thickness = crap_thickness + pcb_thickness + screen_thickness;
case_radius = 1;
full_height = case_height + 2*wall_thickness;
full_width = case_width + 2*wall_thickness;

top_buttons_y = 34.5;
bottom_buttons_y = 19.5;
outer_buttons_x = 6.5;
inner_buttons_x = 13.5;

button_positions = [
    [outer_buttons_x, bottom_buttons_y],
    [outer_buttons_x, top_buttons_y],
    [inner_buttons_x, top_buttons_y],
];

button_radius = 2.5;

speaker_pos = [63, 15];
speaker_hole_radius = 0.75;


charge_hole_to_top = 13;
charge_hole_to_pcb_back = 3.5;
charge_led_to_top = 4;
charge_led_to_pcb_back = 2.5;

power_thickness = 8.5;
power_height = 15;

buttons_inset = 3;

module pcb() {
    translate([-pcb_width/2, -case_height/2, crap_thickness]) color("green") cube([pcb_width, pcb_height, pcb_thickness]);

}

module pcb_holder() {
    rotate([90, 0, 0]) translate([0, 0, -5]) linear_extrude(10) polygon([
        [0,0],
        [0,-4],
        [2,-1],
        [2,0],
    ]);
}

module case_shape_2d() {
    square([full_width-2*case_radius, full_height], center=true);
    square([full_width, full_height-2*case_radius], center=true);
    for (y_sign=[-1,1]) for (x_sign=[-1,1]) {
        translate([x_sign*(full_width/2-case_radius), y_sign*(full_height/2-case_radius)]) circle(case_radius);
    }
}

module back() {

    translate([0, 0, -floor_thickness]) linear_extrude(floor_thickness) {
        difference() {
            case_shape_2d();
            for (x_sign=[-1,1]) {
                translate([x_sign*(full_width/2-screw_holder_radius), -1*(full_height/2-screw_holder_radius)]) circle(screw_hole_radius);
            }
        }
    }
    for (x_sign=[-1,1]) scale([x_sign,1])
        translate([-full_width/2+screw_holder_radius, case_height/2 + screw_holder_radius, -floor_thickness]) linear_extrude(floor_thickness) difference() {
        bunny_ear();
        circle(screw_hole_radius);
    }

}

module button_hole() {
    translate([0,2]) circle(button_radius);
    translate([0,0]) circle(button_radius);
    translate([0,1]) square([2*button_radius, 2], center=true);
}

module button_holes() {
    
    for (x_sign=[-1,1]) scale([x_sign, 1]) for (button_pos=button_positions) {
        translate([-case_width/2, -case_height/2]) translate(button_pos) button_hole();
    }

}

module speaker_holes() {
    
    translate([-case_width/2, -case_height/2]) translate(speaker_pos) {
        circle(speaker_hole_radius);
        for (i=[1:6]) {
            angle = (360*i)/6;
            translate([3*cos(angle),3*sin(angle)]) circle(speaker_hole_radius);
        }
    }
}

module buttons_area_2d(padding=0) {
    for (scale_x=[-1,1]) scale([scale_x,1]) polygon([
        [20-padding, 5+padding],
        [20-padding, -6-padding/sqrt(2)],
        [31-padding,-17-padding/sqrt(2)],
        [31-padding,-30-padding],
        [case_width/2+wall_thickness, -30-padding],
        [case_width/2+wall_thickness, 5+padding],
    ]);
    //translate([29, -30]) square([case_width/2-29,20]);
    //translate([20, -10]) square([case_width/2-20,20]);
}

module raised_area_2d(padding=0) {
    for (scale_x=[-1,1]) scale([scale_x,1]) polygon([
        [20-padding, -6-padding/sqrt(2)],
        [31-padding,-17-padding/sqrt(2)],
        [31-padding,-32+padding],
        
        [-1,-32+padding],
        [-1,32-padding],
        [20-padding,32-padding],
    ]);
}

module viewport() {
    translate([0, case_height/2-viewport_to_screen_top-battery_padding-viewport_height/2]) square([viewport_width,viewport_height], center=true);
}

module front_plate() {
    
    translate([0,0,case_thickness-buttons_inset]) linear_extrude(floor_thickness+buttons_inset) difference() {
        raised_area_2d();
        raised_area_2d(wall_thickness);
    }
    
    translate([0,0,case_thickness]) linear_extrude(floor_thickness) difference() {
        raised_area_2d();
        speaker_holes();
        viewport();
    }
    
    translate([0,0,case_thickness-buttons_inset+floor_thickness]) linear_extrude(buttons_inset) difference() {
        intersection() {
            raised_area_2d(-wall_thickness);
            square([62,case_height],center=true);
        }
        raised_area_2d();
    }
}

module side_walls_2d()
{
    for (x_sign=[-1,1]) scale([x_sign, 1]) translate([-case_width/2-wall_thickness, -full_height/2+case_radius, 0]) square([wall_thickness, full_height-2*case_radius]);
}

module walls() {
    linear_extrude(case_thickness-buttons_inset, convexity=4) {
        for (y_sign=[-1,1]) scale([1,y_sign]) translate([-full_width/2+case_radius, -case_height/2-wall_thickness, 0]) square([full_width-2*case_radius, wall_thickness]);
      
        
        side_walls_2d();

        for (x_sign=[-1,1]) for (y_sign=[-1,1]) scale([x_sign,y_sign]){
            difference() {
                translate([-full_width/2+case_radius,-full_height/2+case_radius]) circle(case_radius);
                translate([-case_width/2,-case_height/2]) square(2*case_radius);
            }
        }    
    }
    
    linear_extrude(case_thickness-buttons_inset, convexity=2) {
        side_walls_2d();
    }
}

module bunny_ear() {
    circle(screw_holder_radius);
    translate([0,-screw_holder_radius/2]) square([2*screw_holder_radius, screw_holder_radius], center=true);
}

module front() {
    difference() {
        walls();
        translate([case_width/2, case_height/2-charge_hole_to_top, crap_thickness - charge_hole_to_pcb_back]) cube([10,12,6],center=true);
        translate([case_width/2+wall_thickness, case_height/2-charge_led_to_top, crap_thickness - charge_led_to_pcb_back]) cube([wall_thickness,2,2], center=true);
        translate([-case_width/2, -case_height/2+pcb_height+power_height/2, crap_thickness-power_thickness/2]) cube([5,power_height,power_thickness], center=true);
    }
    
    linear_extrude(screw_holder_height) for (x_sign=[-1,1]) scale([x_sign,1]) {
        difference() {
            union() {
                translate([-full_width/2+screw_holder_radius,-full_height/2+screw_holder_radius]) circle(screw_holder_radius);
            }
            
                translate([-full_width/2+screw_holder_radius,-full_height/2+screw_holder_radius]) circle(screw_insert_radius);
        }
    }
    
    for (x_sign=[-1,1]) scale([x_sign,1]){
        translate([-full_width/2+screw_holder_radius,-full_height/2+screw_holder_radius, screw_insert_height]) cylinder(h=screw_holder_height-screw_insert_height, r=screw_holder_radius);
        
        translate([-case_width/2, -case_height/2, screw_holder_height]) linear_extrude(2*screw_holder_radius, scale=0) translate([-wall_thickness+screw_holder_radius, -wall_thickness+screw_holder_radius]) circle(screw_holder_radius);
    }
    
    for (x_sign=[-1,1]) scale([x_sign,1])
        translate([-full_width/2+screw_holder_radius, case_height/2 + screw_holder_radius]) linear_extrude(screw_holder_height) difference() {
        bunny_ear();
        circle(screw_insert_radius);
    }
    
    for (x_sign=[-1,1]) scale([x_sign,1])
        translate([-full_width/2+screw_holder_radius, case_height/2 + screw_holder_radius, screw_insert_height]) linear_extrude(screw_holder_height-screw_insert_height) {
        bunny_ear();
    }
    
    for (x_sign=[-1,1]) scale([x_sign,1])
        translate([-full_width/2+screw_holder_radius, full_height/2, screw_holder_height]) linear_extrude(2*screw_holder_radius, scale=0) translate([0, -wall_thickness+screw_holder_radius]) {
        bunny_ear();
    }
    
    for (x_sign=[-1,1]) scale([x_sign, 1]) translate([-case_width/2, -case_height/2+pcb_height/2, crap_thickness+pcb_thickness]) scale([1,1,-1]) pcb_holder();
        
    translate([0, -case_height/2, crap_thickness+pcb_thickness]) rotate([0,0,90]) scale([1,1,-1]) pcb_holder();
    
    for (x_sign=[-1,1]) scale([x_sign,1]) {
    translate([-pcb_width/2, pcb_height-case_height/2, crap_thickness]) cube([10, wall_thickness, case_thickness-crap_thickness-buttons_inset]);
    translate([-pcb_width/2, pcb_height-case_height/2-wall_thickness, crap_thickness+pcb_thickness]) cube([10, 2*wall_thickness, case_thickness-(crap_thickness+pcb_thickness)-buttons_inset]);
    }
    
    translate([0,0,case_thickness-buttons_inset]) linear_extrude(floor_thickness) difference() {
        case_shape_2d();
        //translate([0, case_height/2-viewport_to_screen_top-battery_padding-viewport_height/2]) square([viewport_width,viewport_height], center=true);
        raised_area_2d();
        button_holes();
    }
    
    
}

// pcb();
color("orange") back();
rotate([180, 0, 0]) front();
rotate([180, 0, 0]) color("purple") front_plate();
cube([20,wall_thickness,4]);