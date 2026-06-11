// Focus Totem — sandwich case (bottom tray + switch plate)
// All feature coordinates are in board-local mm: (0,0) is the top-left
// corner of the PCB, matching the KiCad layout. Render one part at a time:
//   openscad -D 'part="plate"'  -o focus-totem-plate.stl  focus-totem.scad
//   openscad -D 'part="bottom"' -o focus-totem-bottom.stl focus-totem.scad

part = "plate";          // "plate" or "bottom"

board_w   = 84;          // PCB outline
board_h   = 72;
board_t   = 1.6;
tol       = 0.4;         // print tolerance, per side

wall      = 2;
floor_t   = 2;
plate_t   = 1.5;
standoff  = 4;           // floor to PCB underside
gap       = 5;           // PCB top to plate underside (MX body)

post_d    = 6;           // standoff / boss outer diameter
m3_clear  = 3.4;
m3_tap    = 2.6;
cbore_d   = 6;           // screw-head counterbore
cbore_h   = 1.4;

sw_cut    = 14;          // MX plate cut-out
enc_d     = 13;          // encoder body + bush clearance
usb_w     = 11;
usb_h     = 5;

holes     = [[4.5,4.5],[79.5,4.5],[4.5,67.5],[79.5,67.5]];
switches  = [[15,32],[34,32],[53,32]];
encoder   = [65.25,12.5];
oled_pos  = [19,5];      // OLED display window
oled_size = [28,13];
usb_x     = 30;          // USB sits on the +y edge

shell_h   = floor_t + standoff + board_t + gap;   // walls reach the plate
$fn = 48;

module screws(d, z0, z1) {
    for (h = holes) translate([h[0], h[1], z0]) cylinder(h = z1 - z0, d = d);
}

module plate() {
    difference() {
        cube([board_w, board_h, plate_t]);
        for (s = switches)
            translate([s[0] - sw_cut/2, s[1] - sw_cut/2, -1]) cube([sw_cut, sw_cut, plate_t + 2]);
        translate([encoder[0], encoder[1], -1]) cylinder(h = plate_t + 2, d = enc_d);
        translate([oled_pos[0], oled_pos[1], -1]) cube([oled_size[0], oled_size[1], plate_t + 2]);
        screws(m3_clear, -1, plate_t + 1);
    }
    // bosses hang under the plate and take the screw thread
    for (h = holes)
        translate([h[0], h[1], -gap]) difference() {
            cylinder(h = gap, d = post_d);
            translate([0, 0, -1]) cylinder(h = gap + 2, d = m3_tap);
        }
}

module bottom() {
    difference() {
        union() {
            difference() {
                translate([-wall, -wall, 0]) cube([board_w + 2*wall, board_h + 2*wall, shell_h]);
                translate([-tol, -tol, floor_t]) cube([board_w + 2*tol, board_h + 2*tol, shell_h + 1]);
                // USB slot through the +y wall
                translate([usb_x - usb_w/2, board_h - 1, floor_t + standoff + board_t - 1])
                    cube([usb_w, wall + 2, usb_h]);
            }
            for (h = holes) translate([h[0], h[1], floor_t]) cylinder(h = standoff, d = post_d);
        }
        screws(m3_clear, -1, shell_h + 1);
        screws(cbore_d, -1, cbore_h);    // recess the screw heads under the floor
    }
}

if (part == "plate") plate();
else bottom();
