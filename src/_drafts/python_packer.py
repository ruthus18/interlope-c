import struct

# Define our data
id_number = 12345
temperature = 98.6
name = "John"

# Pack the data using struct
# Format:
# 'i' - 4-byte integer
# 'f' - 4-byte float
# '4s' - 4-byte character array (string)
format_string = 'if4s'

# Ensure the string is exactly 4 bytes (pad with nulls if needed)
name_bytes = name.encode('utf-8')[:4].ljust(4, b'\0')

# Pack the data into binary format
packed_data = struct.pack(format_string, id_number, temperature, name_bytes)

# Write to a binary file
with open('data.bin', 'wb') as file:
    file.write(packed_data)

print(f"Data packed and written to data.bin")
print(f"Packed size: {len(packed_data)} bytes")
print(f"Data: ID={id_number}, Temp={temperature}, Name={name}")