# TableObjectManager
This program detects put/taken objects on a workspace while detecting human regions.

# License

https://github.com/AtsushiHashimoto/TableObjectManager/blob/master/LICENSE

- /src/modules/OpenCV/include/Labeling.h is provided by IMURA Masataka under the BSD 2-Clause License.
# Quick Start by Nvidia-Docker
- install&run docker and nvidia-docker
- pull docker image
    docker pull atsushihashimoto/table-object-manager
- run the image 
    cd directory/where/video/files/are
    nvidia-docker run -ti -v ./:/root/data atsushihashimoto/table-object-manager /bin/bash
- edit & execute sample script
    # vim sample.sh
    # sh sample.sh video.mpeg
- check results

# Prerequisites
- boost(>=1.54)
- opencv2 (>=2.4.9)
 1. **For use with GPU, COMPILE opencv2 ON YOUR COMPUTER with the "WITH_CUDA"** option (do not use .exe pre-compiled kit). Recommended setting is 'x64'. ('Win32' is not carefully maintained.) 
- CUDA (>=6)
- Vradimir's graph cut implementation (MaxFlow >3.04) http://pub.ist.ac.at/~vnk/software.html
 1. put block.h, graph.h, instances.inc into src/modules/Core/include
 2. put graph.cpp maxflow.cpp into src/modules/Core/src
 3. add #include "instances.inc" in the last line of graph.cpp
 
    The above graph cut implementation is excluded from our repository because it is distributed under the GNU license.
    Please do put above files in the appropriate directory as directed.

# How to compile Library
## on Windows
- open /src/modules/VisualStudioBuild/skl_modules_vs2012.sln
- edit property manager (if you don't find property manager in your window, see http://stackoverflow.com/questions/10179201/cannot-find-property-manager-option-in-visual-studio-not-express-version )
 1. open property manager > SKLCore2012 > Debug > skl2_meta
 2. open 'user macro'.
 3. modify paths along with your computer's setting.
  - SKLRoot: the location of /src
  - OpenCVVersion / OpenCVDir: version and location of opencv2.X
  - BoostDir: location of boost
- install NVIDIA Nsight Visual Studio Edition
 http://www.nvidia.co.jp/object/nsight.html 
- compile it, then compile GPU project
 1. open /src/modules/OpenCVGPU/VisualStudioProject/SKLOpenCVGPU_vs2012.vcxproj WITH TEXT EDITOR! 
 2. replace "CUDA 7.5" to your version (e.g. "CUDA 8.0")
 3. save and close the editor.
- open skl_modules_with_cuda_vs2012.sln and compile
 - If cuda 8.0.props or other files are not found and cannot open SKLOpenCVGPU.vcxproj in the solution file, it is highly expected that you failed to install the cuda toolkit properly. Check "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v8.0\extras\visual_studio_integration\MSBuildExtensions" (v8.0 should be replaced to your version) and copy files to proper directories.

# Examples
## How to compile Examples
- open /examples/path/to/target/*.sln and compile.

## TableObjectManager
A program that record human's access to objects. It automatically assign ID to each detected foreground region. You can get regional images by ID until it is removed from patch_model. In each frame, the program provides following state of objects by IDs of vector.

- put objects: objects put on the table. (ID is assinged to the region at this frame).
- taken objects: objects taken from the table (Once detected as taken, ID is removed in the next frame. After that, you cannot access to this object region.)
- human body region: the region that is judged as human body part.
- hidden objects: objects hidden by the human body part. 
- newly_hidden_objects: hidden objects that was not hidden in the previous frame.
- reappeared_objects: objects that was hidden in the previous frame but not in the current frame.

# How to run the compiled example.
- From command line prompt, execute .exe file with --help or -h.

# Citing Table Object Manager
The algorithm of this program consists of following papers. Please refer any of them when you use this algorithm.

1. /src/OpenCV{,GPU}/{include,src}/TexCut{.h,.cpp}

    ```
    @article{橋本敦史2011texcut,
      title={TexCut: GraphCut を用いたテクスチャの比較による背景差分},
      author={橋本敦史 and 舩冨卓哉 and 中村和晃 and 椋木雅之 and 美濃導彦},
      journal={電子情報通信学会論文誌 D},
      volume={94},
      number={6},
      pages={1007--1016},
      year={2011},
      publisher={The Institute of Electronics, Information and Communication Engineers}
    }
    ```

2. /src/OpenCV/{include,src}PatchModel{.h,.cpp}
3. /src/OpenCV{,GPU}/{include,src}/TableObjectManger\*{.h,.cpp}

    ```
    @article{橋本敦史2012机上物体検出を対象とした接触理由付けによる誤検出棄却,
      title={机上物体検出を対象とした接触理由付けによる誤検出棄却},
      author={橋本敦史 and 舩冨卓哉 and 中村和晃 and 美濃導彦},
      journal={電子情報通信学会論文誌 D},
      volume={95},
      number={12},
      pages={2113--2123},
      year={2012},
      publisher={The Institute of Electronics, Information and Communication Engineers}
    }
    ```
    
English explanation is provided in my doctor thesis.

    http://www.mm.media.kyoto-u.ac.jp/old/research/thesis/2012/d/a_hasimoto/a_hasimoto.pdf

