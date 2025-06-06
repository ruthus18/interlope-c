from contextlib import contextmanager
import enum
import sqlmodel
from sqlmodel import SQLModel, Field, create_engine


class BaseData(SQLModel):

    def add(self):
        with get_session() as session:
            session.add(self)
            session.commit()


class ObjectType(enum.StrEnum):
    STATIC = 'STATIC'
    RIGID = 'RIGID'
    DOOR = 'DOOR'

class Object(BaseData, table=True):
    id: int = Field(None, primary_key=True)
    uid: str = Field(index=True)
    type: ObjectType
    name: str | None

    model_id: int | None = None
    physics_id: int | None = None


class Model(sqlmodel.SQLModel, table=True):
    id: int = Field(None, primary_key=True)
    meshset: str
    textures: str
    slots_count: int


class PhysicsType(enum.StrEnum):
    BOX = 'BOX'
    SPHERE = 'SPHERE'
    CAPSULE = 'CAPSULE'

class Physics(sqlmodel.SQLModel, table=True):
    id: int = Field(None, primary_key=True)
    type: PhysicsType
    size: str
    mass: float = 0.0



engine = sqlmodel.create_engine('sqlite:///data/gamedb.sqlite')


@contextmanager
def get_session():
    with sqlmodel.Session(engine) as session:
        yield session


def apply_schema():
    sqlmodel.SQLModel.metadata.create_all(engine)


def create_object(
    obj: Object,
    model: Model = None,
    physics: Physics = None
):
    match obj.type:
        case ObjectType.STATIC:
            with get_session() as session:
                _get_or_create_model(model, session)
                obj.model_id = 

                session.add(obj)
                session.commit()

        case ObjectType.RIGID:
            ...

        case ObjectType.DOOR:
            ...
    
        case _:
            raise NotImplementedError(f'type={obj.type}')


def test():
    apply_schema()
    
    with get_session() as session:
        obj = Object(
            id=2,
            uid='TestFloor',
            type=ObjectType.STATIC,
        )


    with get_session() as session:
        query = sqlmodel.select(Object).where(Object.uid == 'FloorGrid')
        obj = session.exec(query).first()
        print(obj)


if __name__ == '__main__':
    test()
