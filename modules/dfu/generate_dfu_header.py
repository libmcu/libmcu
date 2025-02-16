import struct
import sys
import os
import subprocess
import argparse

MAGIC = 0xC0DE

HEADER_VERSION = 1

DFU_TYPE_LOADER = 2
DFU_TYPE_UPDATOR = 3
DFU_TYPE_APP = 4


def get_git_tag():
    try:
        tag = subprocess.check_output(["git", "describe", "--tags"]).strip().decode('utf-8')
        if tag.startswith('v'):
            tag = tag[1:]
        version_parts = tag.split('.')
        if len(version_parts) != 3:
            raise ValueError("Invalid tag format")
        version_major = int(version_parts[0])
        version_minor = int(version_parts[1])
        version_patch = int(version_parts[2])
        return version_major, version_minor, version_patch
    except Exception as e:
        print(f"Error getting git tag: {e}. Using default version v0.0.0")
        return 0, 0, 0


def get_git_commit_hash():
    try:
        commit_hash = subprocess.check_output(["git", "rev-parse", "--short=8", "HEAD"]).strip().decode('utf-8')
        return bytes.fromhex(commit_hash)
    except Exception as e:
        print(f"Error getting git commit hash: {e}")
        sys.exit(1)


def generate_dfu_header(file_path, vector_addr, dfu_type, output_path, iv, signature):
    datasize = os.path.getsize(file_path)
    version_major, version_minor, version_patch = get_git_tag()
    git_sha = get_git_commit_hash()

    iv_nonce = bytes.fromhex(iv)
    if len(iv_nonce) != 16:
        print("Error: IV must be 16 bytes (32 hex characters) long.")
        sys.exit(1)

    signature_bytes = bytes.fromhex(signature)
    if len(signature_bytes) > 64:
        print("Error: Signature must be at most 64 bytes (128 hex characters) long.")
        sys.exit(1)
    elif len(signature_bytes) < 64:
        signature_bytes = signature_bytes.ljust(64, b'\x00')

    dfu_image_header = {
        'magic': MAGIC,
        'header_version': HEADER_VERSION,
        'datasize': datasize,
        'type': dfu_type,
        'version_major': version_major,
        'version_minor': version_minor,
        'version_patch': version_patch,
        'vector_addr': vector_addr,
        'git_sha': git_sha,
        'signature': signature_bytes,
        'iv_nonce': iv_nonce,
    }

    dfu_image_header_format = '<HHIBBBBI8s64s16s'

    dfu_image_header_binary = struct.pack(
        dfu_image_header_format,
        dfu_image_header['magic'],
        dfu_image_header['header_version'],
        dfu_image_header['datasize'],
        dfu_image_header['type'],
        dfu_image_header['version_major'],
        dfu_image_header['version_minor'],
        dfu_image_header['version_patch'],
        dfu_image_header['vector_addr'],
        dfu_image_header['git_sha'],
        dfu_image_header['signature'],
        dfu_image_header['iv_nonce']
    )

    with open(output_path, 'wb') as f:
        f.write(dfu_image_header_binary)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate DFU header")
    parser.add_argument('--input', required=True, help="Input file path")
    parser.add_argument('--type', required=True, choices=['app', 'loader', 'updator'], help="DFU type")
    parser.add_argument('--output', required=True, help="Output file path")
    parser.add_argument('--vector', type=lambda x: int(x, 0), default=0, help="Vector address (default: 0)")
    parser.add_argument('--iv', required=False, default='00000000000000000000000000000000', help="Initialization Vector (IV) as a 32-character hex string")
    parser.add_argument('--signature', required=True, help="Signature as a hex string (up to 128 characters)")

    args = parser.parse_args()

    dfu_type_map = {
        'app': DFU_TYPE_APP,
        'loader': DFU_TYPE_LOADER,
        'updator': DFU_TYPE_UPDATOR
    }

    dfu_type = dfu_type_map[args.type]

    generate_dfu_header(args.input, args.vector, dfu_type, args.output, args.iv, args.signature)
