set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(BASE_SOURCES
    ${DIR}/main.cpp
    ${DIR}/Main.cpp
    ${DIR}/MainWindow.cpp
    ${DIR}/MainWindow.h
    ${DIR}/ModelViewer.h
    ${DIR}/ModelViewer.cpp
    ${DIR}/Shader.h
    ${DIR}/Shader.cpp
    ${DIR}/Model.h
    ${DIR}/Model.cpp
    ${DIR}/ModelLoader.h
    ${DIR}/ModelLoader.cpp
    ${DIR}/../Resources/MainWindow.ui
    ${DIR}/../Resources/resources.qrc
    PARENT_SCOPE
)
