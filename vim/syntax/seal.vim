syntax keyword sealKeyword if else while skip stop define return and or not for include
syntax keyword sealType var object
syntax keyword sealBoolean true false null

"syntax match sealNumber  /\v(\d+\.\d+|\d+|\.\d+|0x[0-9A-Fa-f]+)/ 
syntax match sealNumber "\<\d*"
syntax match sealNumber "\<\d*\.d*"

syntax match sealString "\"[^\"\\]*\""

highlight def link sealKeyword Keyword
highlight def link sealType Type
highlight def link sealBoolean Boolean
highlight def link sealNumber Number
highlight def link sealString String

setlocal autoindent
