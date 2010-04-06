package require tcl3d


# Font to be used in the Tk listbox.
set g_listFont {-family {Courier} -size 10}

set g_WinWidth  640
set g_WinHeight 480

set g_LastMousePosX(1) 0
set g_LastMousePosY(1) 0
set g_LastMousePosX(2) 0
set g_LastMousePosY(2) 0

# Spin control: Button 1: View, Button 2: Teapot
set g_fSpinX(1)   0.0
set g_fSpinY(1) -10.0
set g_fSpinX(2)   0.0
set g_fSpinY(2)   0.0

set g_shadowMatrix [tcl3dVector GLfloat 16]
# World position of light source
set g_lightPosition { 2.0 6.0 0.0 1.0 }

set g_bUseStencil false

set X 3
set Y 4
set Z 5
set SIZE 6

#   NX  NY  NZ    X   Y    Z
set g_floorQuad [tcl3dVectorFromArgs GLfloat \
    0.0 1.0 0.0  -5.0 0.0 -5.0 \
    0.0 1.0 0.0  -5.0 0.0  5.0 \
    0.0 1.0 0.0   5.0 0.0  5.0 \
    0.0 1.0 0.0   5.0 0.0 -5.0 \
]

# Show errors occuring in the Togl callbacks.
proc bgerror { msg } {
    tk_messageBox -icon error -type ok -message "Error: $msg\n\n$::errorInfo"
    exit
}

# Print info message into widget a the bottom of the window.
proc PrintInfo { msg } {
    if { [winfo exists .fr.info] } {
        .fr.info configure -text $msg
    }
}

# Update last line of usage messages.
proc UpdateMsg { msgStr } {
    if { [winfo exists .fr.usage] } {
        .fr.usage configure -state normal
        .fr.usage delete end
        .fr.usage insert end $msgStr
        .fr.usage configure -state disabled
    }
}

proc SetMouseInput { btn x y } {
    set ::g_LastMousePosX($btn) $x
    set ::g_LastMousePosY($btn) $y
}

proc GetMouseInput { btn x y } {
    set nXDiff [expr ($x - $::g_LastMousePosX($btn))]
    set nYDiff [expr ($y - $::g_LastMousePosY($btn))]
        
    set ::g_fSpinX($btn) [expr $::g_fSpinX($btn) - $nXDiff]
    set ::g_fSpinY($btn) [expr $::g_fSpinY($btn) - $nYDiff]

    set ::g_LastMousePosX($btn) $x
    set ::g_LastMousePosY($btn) $y
    .fr.toglwin postredisplay
}

proc ToggleStencil {} {
    set ::g_bUseStencil [expr ! $::g_bUseStencil]
    if { $::g_bUseStencil } {
        UpdateMsg "Stencil is ON"
    } else {
        UpdateMsg "Stencil is OFF"
    }
    .fr.toglwin postredisplay
}

proc ChangeLightPos { posInd off } {
    lset ::g_lightPosition $posInd \
         [expr {[lindex $::g_lightPosition $posInd] + $off}]
    .fr.toglwin postredisplay
}

# float fMatrix[16], list fLightPos[4], float fPlane[4]
proc BuildShadowMatrix { fMatrix fLightPos fPlane } {
    # Calculate the dot-product between the plane and the light's position
    set dotp [expr [$fPlane get 0] * [lindex $fLightPos 0] + \
                   [$fPlane get 1] * [lindex $fLightPos 1] + \
                   [$fPlane get 1] * [lindex $fLightPos 2] + \
                   [$fPlane get 3] * [lindex $fLightPos 3]]

    # First column
    set mat(0)  [expr {$dotp - [lindex $fLightPos 0] * [$fPlane get 0]}]
    set mat(4)  [expr {0.0   - [lindex $fLightPos 0] * [$fPlane get 1]}]
    set mat(8)  [expr {0.0   - [lindex $fLightPos 0] * [$fPlane get 2]}]
    set mat(12) [expr {0.0   - [lindex $fLightPos 0] * [$fPlane get 3]}]

    # Second column
    set mat(1)  [expr {0.0   - [lindex $fLightPos 1] * [$fPlane get 0]}]
    set mat(5)  [expr {$dotp - [lindex $fLightPos 1] * [$fPlane get 1]}]
    set mat(9)  [expr {0.0   - [lindex $fLightPos 1] * [$fPlane get 2]}]
    set mat(13) [expr {0.0   - [lindex $fLightPos 1] * [$fPlane get 3]}]

    # Third column
    set mat(2)  [expr {0.0   - [lindex $fLightPos 2] * [$fPlane get 0]}]
    set mat(6)  [expr {0.0   - [lindex $fLightPos 2] * [$fPlane get 1]}]
    set mat(10) [expr {$dotp - [lindex $fLightPos 2] * [$fPlane get 2]}]
    set mat(14) [expr {0.0   - [lindex $fLightPos 2] * [$fPlane get 3]}]

    # Fourth column
    set mat(3)  [expr {0.0   - [lindex $fLightPos 3] * [$fPlane get 0]}]
    set mat(7)  [expr {0.0   - [lindex $fLightPos 3] * [$fPlane get 1]}]
    set mat(11) [expr {0.0   - [lindex $fLightPos 3] * [$fPlane get 2]}]
    set mat(15) [expr {$dotp - [lindex $fLightPos 3] * [$fPlane get 3]}]

    for { set i 0 } { $i < 16 } { incr i } {
        $fMatrix set $i $mat($i)
    }
}

# Find the plane equation given 3 points
# GLfloat plane[4], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3]
proc FindPlane { plane v0 v1 v2 } {
    # Need 2 vectors to find cross product
    set vec0(0) [expr {[$v1 get 0] - [$v0 get 0]}]
    set vec0(1) [expr {[$v1 get 1] - [$v0 get 1]}]
    set vec0(2) [expr {[$v1 get 2] - [$v0 get 2]}]

    set vec1(0) [expr {[$v2 get 0] - [$v0 get 0]}]
    set vec1(1) [expr {[$v2 get 1] - [$v0 get 1]}]
    set vec1(2) [expr {[$v2 get 2] - [$v0 get 2]}]

    # Find cross product to get A, B, and C of plane equation
    $plane set 0 [expr {  $vec0(1) * $vec1(2) - $vec0(2) * $vec1(1)}]
    $plane set 1 [expr {-($vec0(0) * $vec1(2) - $vec0(2) * $vec1(0))}]
    $plane set 2 [expr {  $vec0(0) * $vec1(1) - $vec0(1) * $vec1(0)}]

    $plane set 3 [expr {-([$plane get 0] * [$v0 get 0] + \
                          [$plane get 1] * [$v0 get 1] + \
                          [$plane get 2] * [$v0 get 2])}]
}

proc RenderFloor {} {
    glColor3f 1.0 1.0 1.0
    glInterleavedArrays GL_N3F_V3F 0 $::g_floorQuad
    glDrawArrays GL_QUADS 0 4
}

proc tclCreateFunc { toglwin } {
    glClearColor 0.35 0.53 0.7 1.0
    glEnable GL_LIGHTING
    glEnable GL_LIGHT0
    glEnable GL_DEPTH_TEST

    glMatrixMode GL_PROJECTION
    glLoadIdentity
    gluPerspective 45.0 [expr double($::g_WinWidth)/double($::g_WinHeight)] \
                   0.1 100.0

    # Enable a single OpenGL light.
    set lightAmbient  {0.2 0.2 0.2 1.0}
    set lightDiffuse  {1.0 1.0 1.0 1.0} 
    set lightSpecular {1.0 1.0 1.0 1.0}
    glLightfv GL_LIGHT0 GL_DIFFUSE  $lightDiffuse
    glLightfv GL_LIGHT0 GL_SPECULAR $lightSpecular
    glLightfv GL_LIGHT0 GL_AMBIENT  $lightAmbient
}

proc tclReshapeFunc { toglwin w h } {
    glViewport 0 0 $w $h
    glMatrixMode GL_PROJECTION
    glLoadIdentity
    gluPerspective 45.0 [expr double($w)/double($h)] 0.1 100.0

    set ::g_WinWidth  $w
    set ::g_WinHeight $h
}

proc tclDisplayFunc { toglwin } {
    # Define the plane of the planar surface that we want to cast a shadow on...
    set shadowPlane [tcl3dVector GLfloat 4]
    set v0 [tcl3dVector GLfloat 3]
    set v1 [tcl3dVector GLfloat 3]
    set v2 [tcl3dVector GLfloat 3]

    # To define a plane that matches the floor, we need to 3 vertices from it
    $v0 set 0  [$::g_floorQuad get [expr {0 * $::SIZE + $::X}]]
    $v0 set 1  [$::g_floorQuad get [expr {0 * $::SIZE + $::Y}]]
    $v0 set 2  [$::g_floorQuad get [expr {0 * $::SIZE + $::Z}]]

    $v1 set 0  [$::g_floorQuad get [expr {1 * $::SIZE + $::X}]]
    $v1 set 1  [$::g_floorQuad get [expr {1 * $::SIZE + $::Y}]]
    $v1 set 2  [$::g_floorQuad get [expr {1 * $::SIZE + $::Z}]]

    $v2 set 0  [$::g_floorQuad get [expr {2 * $::SIZE + $::X}]]
    $v2 set 1  [$::g_floorQuad get [expr {2 * $::SIZE + $::Y}]]
    $v2 set 2  [$::g_floorQuad get [expr {2 * $::SIZE + $::Z}]]

    FindPlane $shadowPlane $v0 $v1 $v2

    # Build a shadow matrix using the light's current position and the plane
    BuildShadowMatrix $::g_shadowMatrix $::g_lightPosition $shadowPlane
    
    if { $::g_bUseStencil } {
        glClear [expr $::GL_COLOR_BUFFER_BIT | \
                      $::GL_DEPTH_BUFFER_BIT | \
                      $::GL_STENCIL_BUFFER_BIT]
    } else {
        glClear [expr $::GL_COLOR_BUFFER_BIT | $::GL_DEPTH_BUFFER_BIT]
    }

    # Viewport command is not really needed, but has been inserted for
    # Mac OSX. Presentation framework (Tk) does not send a reshape event,
    # when switching from one demo to another.
    glViewport 0 0 $::g_WinWidth $::g_WinHeight

    # Place the view
    glMatrixMode GL_MODELVIEW
    glLoadIdentity
    glTranslatef 0.0 -2.0 -15.0
    glRotatef [expr -1.0 * $::g_fSpinY(1)] 1.0 0.0 0.0
    glRotatef [expr -1.0 * $::g_fSpinX(1)] 0.0 1.0 0.0

    if { $::g_bUseStencil } {
        # Render the floor to the stencil buffer so we can use it later to trim 
        # the shadow at the floor's edge...

        glEnable GL_STENCIL_TEST
        # Write a 1 to the stencil buffer everywhere we are about to draw
        glStencilFunc GL_ALWAYS 1 0xFFFFFFFF 
        # If a 1 is written to the stencil buffer - simply replace the
        # current value stored there with the 1.
        glStencilOp GL_REPLACE GL_REPLACE GL_REPLACE 

        # Disable writing to the color buffer
        glColorMask GL_FALSE GL_FALSE GL_FALSE GL_FALSE 
        # Disable writing to the depth buffer
        glDepthMask GL_FALSE 

        RenderFloor

        # Re-enable writing to the color buffer
        glColorMask GL_TRUE GL_TRUE GL_TRUE GL_TRUE 
        # Re-enable writing to the depth buffer 
        glDepthMask GL_TRUE 
        glDisable GL_STENCIL_TEST
    }

    # Render the floor...
    RenderFloor

    # Create a shadow by rendering the teapot using the shadow matrix.
    glDisable GL_DEPTH_TEST
    glDisable GL_LIGHTING

    if { $::g_bUseStencil } {
        # Use our stencil to keep the shadow from running off the floor.
        glEnable GL_STENCIL_TEST
        # Only write to areas where the stencil buffer has a 1.
        glStencilFunc GL_EQUAL 1 0xFFFFFFFF
        # Don't modify the contents of the stencil buffer
        glStencilOp GL_KEEP GL_KEEP GL_KEEP

        glEnable GL_BLEND
    }

    glColor3f 0.2 0.2 0.2 ; # Shadow's color
    glPushMatrix
        set shadowMatAsList [tcl3dVectorToList $::g_shadowMatrix 16]
        glMultMatrixf $shadowMatAsList

        # Teapot's position & orientation (needs to use the same
        # transformations used to render the actual teapot)
        glTranslatef 0.0 2.5 0.0
        glRotatef [expr -1.0 * $::g_fSpinY(2)] 1.0 0.0 0.0
        glRotatef [expr -1.0 * $::g_fSpinX(2)] 0.0 1.0 0.0
        glutSolidCube 2.0
    glPopMatrix

    glEnable GL_DEPTH_TEST
    glEnable GL_LIGHTING

    if { $::g_bUseStencil } {
        glDisable GL_BLEND
        glDisable GL_STENCIL_TEST
    }

    # Render the light's position as a sphere...
    glDisable GL_LIGHTING

    glPushMatrix
        # Place the light...
        glLightfv GL_LIGHT0 GL_POSITION $::g_lightPosition

        # Place a sphere to represent the light
        glTranslatef [lindex $::g_lightPosition 0] \
                     [lindex $::g_lightPosition 1] \
                     [lindex $::g_lightPosition 2]

        glColor3f 1.0 1.0 0.5
        glutSolidSphere 0.1 8 8
    glPopMatrix

    glEnable GL_LIGHTING

    # Render normal teapot
    glPushMatrix
        # Teapot's position & orientation
        glTranslatef 0.0 2.5 0.0
        glRotatef [expr -1.0 * $::g_fSpinY(2)] 1.0 0.0 0.0
        glRotatef [expr -1.0 * $::g_fSpinX(2)] 0.0 1.0 0.0
        glutSolidCube 2.0
    glPopMatrix

    $shadowPlane delete
    $v0 delete
    $v1 delete
    $v2 delete
    $toglwin swapbuffers
}

proc Cleanup {} {
    $::g_floorQuad delete
    $::g_shadowMatrix delete

    foreach var [info globals g_*] {
        uplevel #0 unset $var
    }
}

proc ExitProg {} {
    exit
}

frame .fr
pack .fr -expand 1 -fill both
togl .fr.toglwin -width $g_WinWidth -height $g_WinHeight \
                 -stencil true \
                 -double true -depth true \
                 -createproc tclCreateFunc \
                 -reshapeproc tclReshapeFunc \
                 -displayproc tclDisplayFunc 
listbox .fr.usage -font $::g_listFont -height 7
label   .fr.info
grid .fr.toglwin -row 0 -column 0 -sticky news
grid .fr.usage   -row 1 -column 0 -sticky news
grid .fr.info    -row 2 -column 0 -sticky news
grid rowconfigure .fr 0 -weight 1
grid columnconfigure .fr 0 -weight 1

set appTitle "Tcl3D demo: CodeSampler's Planar Shadows"
wm title . $appTitle

# Watch For ESC Key And Quit Messages
wm protocol . WM_DELETE_WINDOW "ExitProg"
bind . <Key-Escape> "ExitProg"
bind . <Key-Up>     "ChangeLightPos 1  0.1"
bind . <Key-Down>   "ChangeLightPos 1 -0.1"
bind . <Key-Left>   "ChangeLightPos 0 -0.1"
bind . <Key-Right>  "ChangeLightPos 0  0.1"
bind . <Key-s>      "ToggleStencil"

bind .fr.toglwin <1>         "SetMouseInput 1 %x %y"
bind .fr.toglwin <B1-Motion> "GetMouseInput 1 %x %y"
bind .fr.toglwin <2>         "SetMouseInput 2 %x %y"
bind .fr.toglwin <B2-Motion> "GetMouseInput 2 %x %y"
bind .fr.toglwin <3>         "SetMouseInput 2 %x %y"
bind .fr.toglwin <B3-Motion> "GetMouseInput 2 %x %y"

.fr.usage insert end "Key-Escape Exit"
.fr.usage insert end "Key-Up|Down    Move light up|down"
.fr.usage insert end "Key-Left|Right Move light left|right"
.fr.usage insert end "Key-s          Toggle stencil usage"
.fr.usage insert end "Mouse-L        Spin the view"
.fr.usage insert end "Mouse-MR       Spin the teapot"
.fr.usage insert end "Stencil messages"
.fr.usage configure -state disabled

UpdateMsg "Stencil is OFF"
tclDisplayFunc .fr.toglwin
PrintInfo [format "Running on %s with a %s (OpenGL %s, Tcl %s)" \
           $tcl_platform(os) [glGetString GL_RENDERER] \
           [glGetString GL_VERSION] [info patchlevel]]
