// Focus Totem — sandwich case (bottom tray + switch plate)
// Feature coordinates are board-local mm: (0,0) is the top-left corner of the
// PCB, matching the KiCad layout. The XIAO mounts on the underside of the PCB
// and hangs into the tray. Four M3 screws drop in from the top through the
// plate and the PCB into tapped standoffs in the tray.
//
// Both parts print flat, no supports: the plate top-up, the tray floor-down.
//   openscad -D 'part="plate"'    -o focus-totem-plate.stl    focus-totem.scad
//   openscad -D 'part="bottom"'   -o focus-totem-bottom.stl   focus-totem.scad
//   openscad -D 'part="assembly"' -o focus-totem.3mf          focus-totem.scad

part = "plate";          // "plate", "bottom" or "assembly"

board_w   = 84;          // PCB outline
board_h   = 72;
board_t   = 1.6;
tol       = 0.4;         // print tolerance, per side

wall      = 2;
floor_t   = 2;
plate_t   = 1.5;
standoff  = 6;           // floor to PCB underside (clears the bottom XIAO)
gap       = 5;           // PCB top to plate underside (MX body)

post_d    = 6;           // standoff outer diameter
m3_clear  = 3.4;
m3_tap    = 2.6;

sw_cut    = 14;          // MX plate cut-out
enc_d     = 13;          // encoder body + bush clearance
led_d     = 5;           // light window over each side LED
usb_w     = 11;

holes     = [[4.5,4.5],[79.5,4.5],[4.5,67.5],[79.5,67.5]];
switches  = [[15,32],[34,32],[53,32]];
encoder   = [65.25,13];
leds      = [[5,50],[78,50]];
oled_pos  = [17,9];      // OLED display window
oled_size = [30,13];
usb_x     = 30;          // USB sits on the +y edge

shell_h   = floor_t + standoff + board_t + gap;   // walls reach the plate
$fn = 48;

module plate() {
    difference() {
        cube([board_w, board_h, plate_t]);
        for (s = switches)
            translate([s[0] - sw_cut/2, s[1] - sw_cut/2, -1]) cube([sw_cut, sw_cut, plate_t + 2]);
        translate([encoder[0], encoder[1], -1]) cylinder(h = plate_t + 2, d = enc_d);
        translate([oled_pos[0], oled_pos[1], -1]) cube([oled_size[0], oled_size[1], plate_t + 2]);
        for (l = leds) translate([l[0], l[1], -1]) cylinder(h = plate_t + 2, d = led_d);
        for (h = holes) translate([h[0], h[1], -1]) cylinder(h = plate_t + 2, d = m3_clear);
        // engraved branding, sits proud-free on the top face
        translate([board_w/2, 62, plate_t - 0.6])
            linear_extrude(0.7) text("FOCUS TOTEM", size = 5, font = "Liberation Sans:style=Bold",
                                     halign = "center", valign = "center");
    }
}

module usb_cutter() {
    // house-shaped opening: square bottom + sharp 45 deg roof so the top self-supports
    translate([usb_x, board_h + wall + 1, floor_t])
        rotate([90, 0, 0])
            linear_extrude(wall + 2)
                polygon([[-usb_w/2, 0], [usb_w/2, 0], [usb_w/2, standoff],
                         [0, standoff + usb_w/2], [-usb_w/2, standoff]]);
}

module bottom() {
    difference() {
        union() {
            difference() {
                translate([-wall, -wall, 0]) cube([board_w + 2*wall, board_h + 2*wall, shell_h]);
                translate([-tol, -tol, floor_t]) cube([board_w + 2*tol, board_h + 2*tol, shell_h + 1]);
                usb_cutter();
            }
            for (h = holes) translate([h[0], h[1], floor_t]) cylinder(h = standoff, d = post_d);
        }
        // tapped blind holes in the standoffs (screws come down from the top)
        for (h = holes) translate([h[0], h[1], floor_t]) cylinder(h = standoff + 0.1, d = m3_tap);
    }
}

// blank PCB stand-in, for the assembled model
module pcb_blank() {
    translate([0, 0, floor_t + standoff]) cube([board_w, board_h, board_t]);
}

// see_through tints the plate for a nicer hero render; the exported model stays solid
module assembly(see_through = false) {
    color([0.20, 0.21, 0.25]) bottom();            // graphite tray
    color([0.09, 0.44, 0.23]) pcb_blank();         // green PCB
    color(see_through ? [0.50, 0.64, 0.86, 0.72] : [0.95, 0.55, 0.15])
        translate([0, 0, shell_h]) plate();        // frosted top, just a hint see-through
}

// --- render-only fittings (keycaps, knob, screen, LEDs) for the hero shot.
// These are real parts, not printed, so they live only in the "hero" render.
module keycap() {
    hull() {
        translate([-9, -9, 0]) cube([18, 18, 0.1]);
        translate([-8, -7, 8]) cube([16, 16, 0.1]);   // slight front taper
    }
}
module fittings() {
    z0 = shell_h + plate_t;                            // plate top
    color([0.13, 0.13, 0.15]) for (s = switches) translate([s[0], s[1], z0]) keycap();
    color([0.10, 0.10, 0.12]) translate([encoder[0], encoder[1], z0]) cylinder(d = 14, h = 16);
    color([0.02, 0.03, 0.06]) translate([oled_pos[0] + 1, oled_pos[1] + 1, z0 - 0.9])
        cube([oled_size[0] - 2, oled_size[1] - 2, 0.8]);
    color([0.35, 0.85, 0.95]) translate([oled_pos[0] + 4, oled_pos[1] + 4, z0 - 0.4])
        cube([oled_size[0] - 8, oled_size[1] - 8, 0.5]);   // a lit display hint
    color([1.0, 0.72, 0.2]) for (l = leds) translate([l[0], l[1], z0 - 0.3]) cylinder(d = led_d - 1, h = 0.6);
}
module hero() { assembly(false); fittings(); }

if (part == "plate") plate();
else if (part == "assembly") assembly(false);      // solid, for the .3mf
else if (part == "render")   assembly(true);       // tinted case, for the fit shot
else if (part == "hero")     hero();               // finished macropad, for the overall shot
else bottom();
