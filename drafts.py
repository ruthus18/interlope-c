import enum
from dataclasses import dataclass


type vec3 = tuple[float, float, float]


@dataclass
class Mesh: ...

@dataclass
class Texture: ...


@dataclass
class Model:
    meshes: list[Mesh]
    textures: list[Texture]

    local_positions: list[vec3]
    local_rotations: list[vec3]


class ObjectType(enum.StrEnum):
    UNKNOWN = enum.auto()
    PLAYER = enum.auto()
    STATIC = enum.auto()
    RIGID_BODY = enum.auto()
    DOOR = enum.auto()
    ITEM = enum.auto()


@dataclass
class Object:
    base_id: str
    instance_id: int
    type: ObjectType

    model: Model
    position: vec3
    rotation: vec3
    local_positions: list[vec3]
    local_rotations: list[vec3]
    