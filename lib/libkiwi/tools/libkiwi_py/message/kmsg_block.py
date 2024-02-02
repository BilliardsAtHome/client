from libkiwi_py.binary.block.block_base import BlockBase
from libkiwi_py.binary.block.header_block_base import HeaderBlockBase


class KMSGBlock(HeaderBlockBase):
    """Message header block"""

    SIGNATURE = "KMSG"
    VERSION = 0x0100  # 1.0

    def __init__(self, blocks: list[BlockBase] = []):
        super().__init__(self.SIGNATURE, self.VERSION, blocks)
