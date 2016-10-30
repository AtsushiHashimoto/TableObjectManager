# TableObjectManager
This program detects put/taken objects on a workspace while detecting human regions.

# License

https://github.com/AtsushiHashimoto/TableObjectManager/blob/master/LICENSE

# Prerequisites
- boost(>=1.54)
- opencv2 (>=2.4.9)
 1. For use with GPU, compile on local computer (otherwise you can use precompiled opencv). Recommended setting is 'x64'. ('Win32' is not carefully maintained.) 
- CUDA (>=6)
- Vradimir's graph cut implementation (MaxFlow >3.04) http://pub.ist.ac.at/~vnk/software.html
 1. put block.h, graph.h, instances.inc into src/modules/Core/include
 2. put graph.cpp maxflow.cpp into src/modules/Core/src
 3. add #include "instances.inc" in the last line of graph.cpp

# Compile Library
## on Windows
- open /src/modules/VisualStudioBuild/skl_modules_vs2012.sln
- edit property sheet
 1. open property manager > SKLCore2012 > Debug > skl2_meta
 2. open 'user macro'.
 3. modify paths along with your computer's setting.
  - SKLRoot: the location of /src
  - OpenCVVersion / OpenCVDir: version and location of opencv2.X
  - BoostDir: location of boost
- compile it, then open skl_modules_with_cuda_vs2012.sln WITH TEXT EDITOR!
 1. replace CUDA7.5 to your version (e.g. CUDA8.0)
 2. save and close the editor.
- open skl_modules_with_cuda_vs2012.sln and compile

# Compile Examples
- open /examples/path/to/target/*.sln and compile.

# How to use
- From command line prompt, execute .exe file with --help or -h.

# Citing Table Object Manager
The algorithm of this program consists of following papers. Please refer any of them when you use this algorithm.

1. /src/OpenCV{,GPU}/{include,src}/TexCut{.h,.cpp}

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

2. /src/OpenCV/{include,src}PatchModel{.h,.cpp}
3. /src/OpenCV{,GPU}/{include,src}/TableObjectManger\*{.h,.cpp}

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
    
English explanation is provided in my doctor thesis.

    http://www.mm.media.kyoto-u.ac.jp/old/research/thesis/2012/d/a_hasimoto/a_hasimoto.pdf

