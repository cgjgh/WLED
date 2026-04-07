"""
WLED Clock Layout Generator
---------------------------
Run this Python script to automatically regenerate the LC_LEDMAP matrix
inside your const_ledclock.h file!
"""

import re
import os

# ==========================================
# ===== USER CONFIGURATION SECTION =========
# ==========================================

# Dimensions
LEDS_PER_SEG = 10
SEP_LEDS = 8

# Hardware snake order configuration
# True if wire starts at the Leftmost digit. 
# False if wire starts at the Rightmost digit.
START_AT_LEFTMOST_DIGIT = False

# Toggle these to mirror the logical placement of each individual digit to account 
# for physical hardware rotation or counter-clockwise segment routing from the back.
FLIP_VERTICALLY = True
FLIP_HORIZONTALLY = False

# The vertical row locations (Y-coordinates) for your separator LEDs.
# These determine the geometric placement of the colon within the matrix.
# For LEDS_PER_SEG=10, the total height is 23, so middle is row 11.
SEP_ROWS = [16, 15, 14, 13, 9, 8, 7, 6]

# Path to the const_ledclock.h file (Dynamically resolves to same directory as script)
script_dir = os.path.dirname(os.path.abspath(__file__))
H_FILE_PATH = os.path.join(script_dir, "const_ledclock.h")

# ==========================================
# ==========================================


DIGIT_WIDTH = LEDS_PER_SEG + 2
DIGIT_HEIGHT = LEDS_PER_SEG * 2 + 3

COLS = 4 * DIGIT_WIDTH + 1
ROWS = DIGIT_HEIGHT

matrix = [[-1 for _ in range(COLS)] for _ in range(ROWS)]

def assign(r, c, val, C_offset):
    rel_c = c - C_offset
    mirror_v_r = (ROWS - 1 - r) if FLIP_VERTICALLY else r
    mirror_h_c = (DIGIT_WIDTH - 1 - rel_c) if FLIP_HORIZONTALLY else rel_c
    abs_c = C_offset + mirror_h_c
    if 0 <= mirror_v_r < ROWS and 0 <= abs_c < COLS:
        matrix[mirror_v_r][abs_c] = val

for digit_idx in range(4):
    if digit_idx < 2:
        C = digit_idx * DIGIT_WIDTH
    else:
        C = digit_idx * DIGIT_WIDTH + 1
    
    if START_AT_LEFTMOST_DIGIT:
        S = digit_idx * (7 * LEDS_PER_SEG)
        if digit_idx >= 2:
            S += SEP_LEDS
    else:
        S = (3 - digit_idx) * (7 * LEDS_PER_SEG)
        if digit_idx < 2:
            S += SEP_LEDS
    
    L = LEDS_PER_SEG
    
    # Calculate base physical index for each segment
    S_TR  = S
    S_T   = S + L
    S_TL  = S + 2*L
    S_BL  = S + 3*L
    S_B   = S + 4*L
    S_BR  = S + 5*L
    S_M   = S + 6*L

    for i in range(L):
        # Segment 1: TR (Row L -> 1)
        assign(L - i, C + (DIGIT_WIDTH - 1), S_TR + i, C)
        # Segment 2: T (Right to Left)
        assign(0, C + L - i, S_T + i, C)
        # Segment 3: TL (Row 1 -> L)
        assign(1 + i, C, S_TL + i, C)
        # Segment 4: BL (Row L+2 -> 2L+1)
        assign(L + 2 + i, C, S_BL + i, C)
        # Segment 5: B (Left to Right)
        assign(2 * L + 2, C + 1 + i, S_B + i, C)
        # Segment 6: BR (Row 2L+1 -> L+2)
        assign(2 * L + 1 - i, C + (DIGIT_WIDTH - 1), S_BR + i, C)
        # Segment 7: M (Right to Left)
        assign(L + 1, C + L - i, S_M + i, C)

# Separator placement always lands geometrically in the center and physically after the 2nd digit
colon_s = 2 * (7 * LEDS_PER_SEG)
c_col = 2 * DIGIT_WIDTH

for i, r in enumerate(SEP_ROWS):
    if 0 <= r < ROWS and i < SEP_LEDS:
        matrix[r][c_col] = colon_s + i

out = ""
for r in range(ROWS):
    line = ",".join(f"{str(x):>3}" for x in matrix[r]) + (", \\" if r < ROWS-1 else "")
    out += line + "\n"

if os.path.exists(H_FILE_PATH):
    with open(H_FILE_PATH, "r") as f:
        text = f.read()

    # Dynamic replacement between known token boundaries
    start_match = re.search(r"#define LC_LEDMAP \\\n", text)
    end_match = re.search(r"\n\n// Next configure", text)

    if start_match and end_match:
        start_idx = start_match.end()
        end_idx = end_match.start()
        
        new_text = text[:start_idx] + out + text[end_idx:]
        with open(H_FILE_PATH, "w") as f:
            f.write(new_text)
        print("Success! The const_ledclock.h file has been updated with the new matrix.")
    else:
        print("Error: Could not find target bounds to inject layout. Check const_ledclock.h formatting.")
else:
    print(f"Error: {H_FILE_PATH} not found.")
