import cbor2
import json

# Create a Python data structure to encode
data = {
    'name': 'John Doe',
    'age': 30,
    'is_student': True,
    'grades': [85, 90, 92]
}

# Encoding (Python data structure to CBOR)
encoded_data = cbor2.dumps(data)

# Decoding (CBOR to Python data structure)
decoded_data = cbor2.loads(encoded_data)

# Print the original and decoded data
print("Original Data:", data)
print("Decoded Data:", decoded_data)

