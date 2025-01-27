#pragma once

// OUTPUT LEVEL INSTRUCTIONS
#define DALI_CMD_OFF                            0x00 // 0 - Switches off lamp(s)
#define DALI_CMD_UP                             0x01 // 1 - Increases lamp(s) illumination level
#define DALI_CMD_DOWN                           0x02 // 2 - Decreases lamp(s) illumination level
#define DALI_CMD_STEP_UP                        0x03 // 3 - Increases the target illumination level by 1
#define DALI_CMD_STEP_DOWN                      0x04 // 4 - Decreases the target illumination level by 1
#define DALI_CMD_RECALL_MAX_LEVEL               0x05 // 5 - Changes the current light output to the maximum level
#define DALI_CMD_RECALL_MIN_LEVEL               0x06 // 6 - Changes the current light output to the minimum level
#define DALI_CMD_STEP_DOWN_AND_OFF              0x07 // 7 - If the target level is zero, lamp(s) are turned off; if the target level is between the min. and max. levels, decrease the target level by one; if the target level is max., lamp(s) are turned off
#define DALI_CMD_ON_AND_STEP_UP                 0x08 // 8 - If the target level is zero, lamp(s) are set to minimum level; if target level is between min. and max. levels, increase the target level by one
#define DALI_CMD_ENABLE_DAPC_SEQUENCE           0x09 // 9 - Indicates the start of DAPC (level) commands
#define DALI_CMD_GO_TO_LAST_ACTIVE_LEVEL        0x0A // 10 - Sets the target level to the last active output level
#define DALI_CMD_GO_TO_SCENE                    0x10 // 16 - Sets a group of lamps to a predefined scene

// CONFIGURATION INSTRUCTIONS
#define DALI_CMD_DALI_RESET                     0x20 // 32 - Configures all variables back to their Reset state
#define DALI_CMD_STORE_ACTUAL_LEVEL_IN          0x21 // 33 - Stores the actual level value into Data Transfer Register 0 (DTR0)
#define DALI_CMD_SAVE_PERSISTENT_VARIABLES      0x22 // 34 - Stores all variables into Nonvolatile Memory (NVM)
#define DALI_CMD_SET_OPERATING_MODE             0x23 // 35 - Sets the operating mode to the value listed in DTR0
#define DALI_CMD_RESET_MEMORY_BANK              0x24 // 36 - Resets the memory bank identified by DTR0 (memory bank must be implemented and unlocked)
#define DALI_CMD_IDENTIFY_DEVICE                0x25 // 37 - Instructs a control gear to run an identification procedure
#define DALI_CMD_SET_MAX_LEVEL                  0x2A // 42 - Configures the control gear's maximum output level to the value stored in DTR0
#define DALI_CMD_SET_MIN_LEVEL                  0x2B // 43 - Configures the control gear's minimum output level to the value stored in DTR0
#define DALI_CMD_SET_SYSTEM_FAILURE_LEVEL       0x2C // 44 - Sets the control gear's output level in the event of a system failure to the value stored in DTR0
#define DALI_CMD_SET_POWER_ON_LEVEL             0x2D // 45 - Configures the output level upon power-up based on the value of DTR0
#define DALI_CMD_SET_FADE_TIME                  0x2E // 46 - Sets the fade time based on the value of DTR0
#define DALI_CMD_SET_FADE_RATE                  0x2F // 47 - Sets the fade rate based on the value of DTR0
#define DALI_CMD_SET_EXTENDED_FADE_TIME         0x30 // 48 - Sets the extended fade rate based on the value of DTR0; used when fade time = 0

#define DALI_CMD_SET_SCENE                      0x40 // 64-79 - Configures scene 'x' based on the value of DTR0
#define DALI_CMD_REMOVE_FROM_SCENE              0x50 // 80-95 - Removes one of the control gears from a scene
#define DALI_CMD_ADD_TO_GROUP                   0x60 // 96-111 - Adds a control gear to a group
#define DALI_CMD_REMOVE_FROM_GROUP              0x70 // 112-127 - Removes a control gear from a group
#define DALI_CMD_SET_SHORT_ADDRESS              0x80 // 128 - Sets a control gear's short address to the value of DTR0
#define DALI_CMD_ENABLE_WRITE_MEMORY            0x81 // 129 - Allows writing into memory banks

// QUERY INSTRUCTIONS
#define DALI_CMD_QUERY_STATUS                   0x90 // 144 - Determines the control gear's status based on a combination of gear properties
#define DALI_CMD_QUERY_CONTROL_GEAR_PRESENT     0x91 // 145 - Determines if a control gear is present
#define DALI_CMD_QUERY_LAMP_FAILURE             0x92 // 146 - Determines if a lamp has failed
#define DALI_CMD_QUERY_LAMP_POWER_ON            0x93 // 147 - Determines if a lamp is On
#define DALI_CMD_QUERY_LIMIT_ERROR              0x94 // 148 - Determines if the requested target level has been modified due to max. or min. level limitations
#define DALI_CMD_QUERY_RESET_STATE              0x95 // 149 - Determines if all NVM variables are in their Reset state
#define DALI_CMD_QUERY_MISSING_SHORT_ADDRESS    0x96 // 150 - Determines if a control gear's address is equal to 0xFF
#define DALI_CMD_QUERY_VERSION_NUMBER           0x97 // 151 - Returns the corresponding IEC standard number
#define DALI_CMD_QUERY_CONTENT_DTR0             0x98 // 152 - Returns the value of DTR0
#define DALI_CMD_QUERY_DEVICE_TYPE              0x99 // 153 - Determines the device type supported by the control gear
#define DALI_CMD_QUERY_PHYSICAL_MINIMUM         0x9A // 154 - Returns the minimum light output that the control gear can operate at
#define DALI_CMD_QUERY_POWER_FAILURE            0x9B // 155 - Determines if an external power cycle occurred
#define DALI_CMD_QUERY_CONTENT_DTR1             0x9C // 156 - Returns the value of DTR1
#define DALI_CMD_QUERY_CONTENT_DTR2             0x9D // 157 - Returns the value of DTR2
#define DALI_CMD_QUERY_OPERATING_MODE           0x9E // 158 - Determines the control gear's operating mode
#define DALI_CMD_QUERY_LIGHT_SOURCE_TYPE        0x9F // 159 - Returns the control gear's type of light source
#define DALI_CMD_QUERY_ACTUAL_LEVEL             0xA0 // 160 - Returns the control gear's actual power output level
#define DALI_CMD_QUERY_MAX_LEVEL                0xA1 // 161 - Returns the control gear's maximum output setting
#define DALI_CMD_QUERY_MIN_LEVEL                0xA2 // 162 - Returns the control gear's minimum output setting
#define DALI_CMD_QUERY_POWER_ON_LEVEL           0xA3 // 163 - Returns the value of the intensity level upon power-up
#define DALI_CMD_QUERY_SYSTEM_FAILURE_LEVEL     0xA4 // 164 - Returns the value of the intensity level due to a system failure
#define DALI_CMD_QUERY_FADE_TIME_FADE_RATE      0xA5 // 165 - Returns a byte in which the upper nibble is equal to the fade time value and the lower nibble is the fade rate value
#define DALI_CMD_QUERY_MANUFACTURER_SPECIFIC_MODE 0xA6 // 166 - Returns a 'YES' when the operating mode is within the range of 0x80 //  0xFF
#define DALI_CMD_QUERY_NEXT_DEVICE_TYPE         0xA7 // 167 - Determines if the control gear has more than one feature, and if so, return the first/next device type or feature
#define DALI_CMD_QUERY_EXTENDED_FADE_TIME       0xA8 // 168 - Returns a byte in which bits 6-4 is the value of the extended fade time multiplier and the lower nibble is the extended fade time base
#define DALI_CMD_QUERY_CONTROL_GEAR_FAILURE     0xAA // 170 - Determines if a control gear has failed
#define DALI_CMD_QUERY_SCENE_LEVEL              0xB0 // 176 - Returns the level value of scene 'x'
#define DALI_CMD_QUERY_GROUPS_0_7               0xC0 // 192 - Returns a byte in which each bit represents a member of a group. A '1' represents a member of the group
#define DALI_CMD_QUERY_GROUPS_8_15              0xC1 // 193 - Returns a byte in which each bit represents a member of a group. A '1' represents a member of the group
#define DALI_CMD_QUERY_RANDOM_ADDRESS_H         0xC2 // 194 - Returns the upper byte of a randomly generated address
#define DALI_CMD_QUERY_RANDOM_ADDRESS_M         0xC3 // 195 - Returns the high byte of a randomly generated address
#define DALI_CMD_QUERY_RANDOM_ADDRESS_L         0xC4 // 196 - Returns the low byte of a randomly generated address
#define DALI_CMD_READ_MEMORY_LOCATION           0xC5 // 197 - Returns the content of the memory location stored in DTR0 that is located within the memory bank listed in DTR1

// DT8 COMMANDS
#define DALI_CMD_SET_TEMPORARY_X_COORDINATE     0xE0 // 224 - Set Temporary X-Coordinate (DTR = LSB, DTR1 = MSB)
#define DALI_CMD_SET_TEMPORARY_Y_COORDINATE     0xE1 // 225 - Set Temporary Y-Coordinate (DTR = LSB, DTR1 = MSB)
#define DALI_CMD_ACTIVATE                       0xE2 // 226 - Activate
#define DALI_CMD_X_COORDINATE_STEP_UP           0xE3 // 227 - X-Coordinate Step up
#define DALI_CMD_X_COORDINATE_STEP_DOWN         0xE4 // 228 - X-Coordinate Step down
#define DALI_CMD_Y_COORDINATE_STEP_UP           0xE5 // 229 - Y-Coordinate Step up
#define DALI_CMD_Y_COORDINATE_STEP_DOWN         0xE6 // 230 - Y-Coordinate Step down
#define DALI_CMD_SET_TEMPORARY_COLOUR_TEMPERATURE 0xE7 // 231 - Set Temporary Colour Temperature (DTR = Mirek LSB, DTR1 = Mirek MSB)
#define DALI_CMD_COLOUR_TEMPERATURE_STEP_COOLER 0xE8 // 232 - Colour Temperature Step Cooler
#define DALI_CMD_COLOUR_TEMPERATURE_STEP_WARMER 0xE9 // 233 - Colour Temperature Step Warmer
#define DALI_CMD_SET_TEMPORARY_PRI_DIM_LEVEL    0xEA // 234 - Set Temporary Primary N Dim Level
#define DALI_CMD_SET_TEMPORARY_RGB_DIM_LEVEL    0xEB // 235 - Set Temporary RGB Dim Level (DTR0 = , DTR1 = , DTR2 = )
#define DALI_CMD_SET_TEMPORARY_WAF_DIM_LEVEL    0xEC // 236 - Set Temporary WAF Dim Level (DTR0 = , DTR1 = , DTR2 = )
#define DALI_CMD_SET_TEMPORARY_RGBWAF_CONTROL   0xED // 237 - Set Temporary RGBWAF Control (DTR0 = 0)
#define DALI_CMD_COPY_REPORT_TO_TEMPORARY       0xEE // 238 - Copy Report To Temporary
// 239
#define DALI_CMD_STORE_TY_PRIMARY_N             0xF0 // 240 - Store TY Primary N (DTR0 = , DTR1 = , DTR2 = , Send twice)
#define DALI_CMD_SET_XY_COORDINATE_PRIMARY      0xF1 // 241 - Store XY Coordinate Primary N (DTR2 = , Send twice)
#define DALI_CMD_STORE_COLOUR_TEMPERATURE_LIMIT 0xF2 // 242 - Store Colour Temperature Limit (DTR0, DTR1, DTR2, Send twice)
#define DALI_CMD_STORE_GEAR_FEATURE_STATUS      0xF3 // 243 - Store Gear Feature Status
// 244
#define DALI_CMD_ASSIGN_COLOUR_TO_LINKED_CHANNEL 0xF5 // 245 - Assign Colour To Linked Channel
#define DALI_CMD_START_AUTO_CALIBRATION         0xF6 // 246 - Start Auto Calibration
#define DALI_CMD_QUERY_GEAR_FEATURES_STATUS     0xF7 // 247 - Query Gear Features Status
#define DALI_CMD_QUERY_COLOUR_STATUS            0xF8 // 248 - Query Colour Status
#define DALI_CMD_QUERY_COLOUR_FEATURES          0xF9 // 249 - Query Colour Features
#define DALI_CMD_QUERY_COLOUR_VALUE             0xFA // 250 - Query Colour Value
#define DALI_CMD_QUERY_RGBWAF_CONTROL           0xFB // 251 - Query RGBWAF Control
#define DALI_CMD_QUERY_ASSIGNED_COLOUR          0xFC // 252 - Query Assigned Colour
// 253
// 254
#define DALI_CMD_QUERY_EXTENDED_VERSION_NUMBER  0xFF // 255 - Returns the version number belonging to the device type or feature

// SPECIAL COMMANDS
#define DALI_CMD_TERMINATE                      0xA1 // 256 - Stops the control gear's initilization
#define DALI_CMD_DTR0_DATA                      0xA3 // 257 - Loads a data byte into DTR0
#define DALI_CMD_INITIALISE                     0xA5 // 258 - Initializes a control gear, command must be issued twice
#define DALI_CMD_RANDOMIZE                      0xA7 // 259 - Generates a random address value, command must be issued twice
#define DALI_CMD_COMPARE                        0xA9 // 260 - Compares the random address variable to the search address variable
#define DALI_CMD_WITHDRAW                       0xAB // 261 - Changes the initialization state to reflect that a control gear had been identified but remains in the initialization state
#define DALI_CMD_PING                           0xAD // 263 - Used by control devices to indicate theirm presence on the bus
#define DALI_CMD_SEARCH_ADDRH                   0xB1 // 264 - Determines if an address is present on the bus
#define DALI_CMD_SEARCH_ADDRM                   0xB3 // 265 - Determines if an address is present on the bus
#define DALI_CMD_SEARCH_ADDRL                   0xB5 // 266 - Determines if an address is present on the bus
#define DALI_CMD_PROGRAM_SHORT_ADDRESS          0xB7 // 267 - Programs a control gear's short address
#define DALI_CMD_VERIFY_SHORT_ADDRESS           0xB9 // 268 - Verifies if a control gear's short address is correct
#define DALI_CMD_QUERY_SHORT_ADDRESS            0xBB // 269 - Queries a control gear's short address
#define DALI_CMD_PHYSICAL_SELECTION             0xBD // 270 -
#define DALI_CMD_ENABLE_DEVICE_TYPE             0xC1 // 272 - Enables a control gear's device type function
#define DALI_CMD_DTR1_DATA                      0xC3 // 273 - Loads a data byte into DTR1
#define DALI_CMD_DTR2_DATA                      0xC5 // 274 - Loads a data byte into DTR2
#define DALI_CMD_WRITE_MEMORY_LOCATION          0xC7 // 275 - Writes data into a specific memory location and returns the value of the data written
#define DALI_CMD_WRITE_MEMORY_LOCATION_NO_REPLY 0xC9 // 276 - Writes data into a specific memory location but does not return a response

