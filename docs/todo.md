misc:
anti aliasing
negative world chunks
16^3 chunks instead of 16^2 x 128 chunks?
chunk load and unload from disk
background terrain gen
async instance gen
only instance gen for chunks that have neighbors
make transparency cache for chunk instance building?
make orthographic matrix depend on render distance
make shadow texture size depend on render distance
mipmap shadows
make better fog
texture interpolation instead of color interpolation
make text renderer draw on near plane
fix blocks placed on chunk boundary issues
inventory
day night cycle
handle resizes
investigate shadow volumes
draw player
player shadow
investigate making glad a static lib
investigate making terrain generation seed-based

premake:
advanced terrain gen - air cutoff with gradient, cave digouts
runescape less strong
extreme hill only sometimes
rivers

final:
water transparency
downsample with intelligent interpolation
neighboring chunks on break refresh a lot
make caves not iterate through every previous cave to run
> DONE
caves
trees
trees mess up friends that have already instanced
incremental chunk generation
incremental chunk building
fix player (collision?) crashing outside of chunk height bounds
stretch octaves
3d perlin

bugs:
need to put a mutex around {the body of the worker thread} and {the instance building}. chunk's backing store might be resized