#!/usr/bin/env python3
"""
Generate a JSON instance file from SID files using pycoreconf model.

This script reads SID files and generates a skeletal JSON instance with:
- All data nodes with default values
- Lists with sample entries (including keys)
- Containers as empty objects
- Only nodes that actually exist in the SID files
"""

import argparse
import json
import sys
from pathlib import Path
from collections import OrderedDict
import pycoreconf


def get_default_value_for_type(yang_type):
    """Return a sensible default value for a YANG type."""
    # Handle enumeration types (stored as dicts in SID files)
    # The dict maps numeric values to string names: {0: 'name1', 1: 'name2'}
    # We need to return the string name, not the numeric value
    if isinstance(yang_type, dict):
        if yang_type:
            # Return the name (value) of the enum with the minimum numeric key
            min_key = min(yang_type.keys())
            return yang_type[min_key]
        return 0

    # Handle YANG types with prefixes (e.g., "yang:counter32", "ietf-yang-types:mac-address")
    base_type = yang_type.split(':')[-1] if ':' in yang_type else yang_type

    # Map base types to appropriate defaults
    # Integer types
    if base_type in ['uint8', 'uint16', 'uint32', 'uint64', 'counter32', 'counter64', 'gauge32', 'gauge64']:
        return 0
    if base_type in ['int8', 'int16', 'int32', 'int64']:
        return 0

    # Boolean
    if base_type == 'boolean':
        return False

    # Decimal
    if base_type in ['decimal64']:
        return 0.0

    # String-like types - use appropriate string defaults
    if base_type == 'string':
        return "0"
    if base_type == 'mac-address' or base_type == 'phys-address':
        return "00:00:00:00:00:00"
    if base_type == 'date-and-time':
        return "1970-01-01T00:00:00Z"
    if base_type in ['yang-identifier', 'hex-string', 'uuid', 'dotted-quad', 'xpath1.0']:
        return "0"

    # Types that should be 0 (treated as numeric)
    if base_type in ['bits', 'binary']:
        return 0

    # Empty type (presence)
    if base_type == 'empty':
        return None

    # Reference types - use numeric 0 instead of empty string
    if base_type in ['identityref', 'leafref', 'instance-identifier']:
        return 0

    # Union - default to 0
    if base_type == 'union':
        return 0

    # For unknown types, default to 0
    return 0


def build_instance(model_sid, add_sample_entries=True):
    """
    Build a JSON instance from the pycoreconf ModelSID.

    Args:
        model_sid: A pycoreconf ModelSID object with parsed SID files
        add_sample_entries: Whether to add sample entries to lists

    Returns:
        OrderedDict representing the JSON instance
    """
    instance = OrderedDict()

    # Build a set of all valid identifiers in the SID files
    valid_identifiers = set(model_sid.sids.keys())

    # Get all data nodes sorted by path depth
    data_nodes = []
    for identifier, sid in model_sid.sids.items():
        namespace = model_sid.namespace.get(identifier)
        if namespace == 'data':
            item_type = model_sid.types.get(identifier, 'void')
            data_nodes.append({
                'identifier': identifier,
                'sid': sid,
                'type': item_type,
                'is_list': str(sid) in model_sid.key_mapping
            })

    # Sort by path depth (number of slashes)
    data_nodes.sort(key=lambda x: x['identifier'].count('/'))

    print(f"Processing {len(data_nodes)} data nodes...")

    for node in data_nodes:
        identifier = node['identifier']
        sid = node['sid']
        item_type = node['type']
        is_list = node['is_list']

        # Skip module namespace items
        if not identifier.startswith('/'):
            continue

        # Parse path
        path_parts = [p for p in identifier.split('/') if p]
        if not path_parts:
            continue

        # Check ALL parent paths to ensure they exist in SID files
        # If any parent doesn't exist, it's a YANG choice/case node that shouldn't be in JSON
        skip_node = False
        for i in range(1, len(path_parts)):
            check_path = '/' + '/'.join(path_parts[:i])
            if check_path not in valid_identifiers:
                # This parent path doesn't exist in SID - skip this entire node
                print(f"  Skipping {identifier}")
                print(f"    Reason: parent '{check_path}' not in SID files")
                skip_node = True
                break

        if skip_node:
            continue

        # Navigate to parent and create the node
        current = instance
        for i, part in enumerate(path_parts[:-1]):
            # Create intermediate node if it doesn't exist (we know it's valid now)
            if part not in current:
                current[part] = OrderedDict()

            # Handle list navigation
            if isinstance(current[part], list):
                # Navigate into first list entry if it exists
                if len(current[part]) > 0 and isinstance(current[part][0], dict):
                    current = current[part][0]
                else:
                    # Can't navigate into empty list - skip this node
                    print(f"  Skipping {identifier} - parent list is empty")
                    current = None
                    break
            elif isinstance(current[part], (dict, OrderedDict)):
                current = current[part]
            else:
                # Parent is a leaf value - can't add children
                print(f"  Skipping {identifier} - parent is a leaf")
                current = None
                break

        if current is None:
            continue

        # Add the leaf/container/list
        leaf_name = path_parts[-1]

        if is_list:
            # Create list with sample entry if requested
            if add_sample_entries:
                entry = OrderedDict()

                # Add keys
                key_sids = model_sid.key_mapping[str(sid)]
                for key_sid in key_sids:
                    if key_sid in model_sid.ids:
                        key_identifier = model_sid.ids[key_sid]
                        key_name = key_identifier.split('/')[-1]
                        key_type = model_sid.types.get(key_identifier, 'string')
                        entry[key_name] = get_default_value_for_type(key_type)

                current[leaf_name] = [entry]
                print(f"  {identifier} -> list with entry")
            else:
                current[leaf_name] = []
                print(f"  {identifier} -> empty list")

        elif item_type == 'void':
            # Container
            if leaf_name not in current:
                current[leaf_name] = OrderedDict()
            print(f"  {identifier} -> container")

        else:
            # Leaf with value
            # Skip problematic reference types that pycoreconf can't handle without actual data
            base_type = item_type.split(':')[-1] if ':' in item_type else item_type
            if base_type in ['identityref', 'leafref', 'instance-identifier']:
                print(f"  {identifier} -> skipping ({item_type}) - reference type needs actual data")
                continue

            default_value = get_default_value_for_type(item_type)
            current[leaf_name] = default_value
            print(f"  {identifier} -> leaf ({item_type})")

    return instance


def main():
    parser = argparse.ArgumentParser(
        description="Generate a JSON instance file from SID files using pycoreconf",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        '-f', '--files',
        nargs='+',
        required=True,
        help='List of .sid files'
    )
    parser.add_argument(
        '-o', '--output',
        required=True,
        help='Output JSON file path'
    )
    parser.add_argument(
        '--no-sample-entries',
        action='store_true',
        help='Do not add sample entries to lists (create empty arrays)'
    )

    args = parser.parse_args()

    # Load SID files using pycoreconf
    print(f"Loading SID files: {args.files}")
    try:
        model = pycoreconf.CORECONFModel(args.files)
    except Exception as e:
        print(f"Error loading SID files: {e}")
        return 1

    # Build instance
    add_samples = not args.no_sample_entries
    instance = build_instance(model, add_sample_entries=add_samples)

    # Write to file
    print(f"\nWriting to {args.output}")
    with open(args.output, 'w') as f:
        json.dump(instance, f, indent=2)

    print("Done!")
    return 0


if __name__ == '__main__':
    sys.exit(main())
