* 参考1：[MSBuild (Visual C++) Overview](https://msdn.microsoft.com/en-us/library/ee662426.aspx)
* 参考2：[Walkthrough: Using MSBuild to Create a Visual C++ Project](https://msdn.microsoft.com/en-us/library/dd293607.aspx)

### 一。跟标准 MSBuild 的区别

#### 1. ProjectConfiguration（Item）
* 必须有 ProjectConfiguration 这个 Item，它指定 project 是在哪种 Configuration 和哪种 Platform 下来 build 的。
* 它的值（即 Include 属性）必须是类似这种："Debug|Win32"，"Release|x64"。
* VC++ 更常用 Item 来定义 build 过程中的一些设置，而不是常用 Property，比如上述 ProjectConfiguration 就是一个 Item
* 注意：运行 MSBuild 时如果不指定参数，则它默认按照 "Debug|Win32" 来进行 build。如果想 build release 版，则运行时加上参数：`/p:Configuration=Release`，并且保证你的 project file 里 ProjectConfiguration 这个 Item 的值里包括 "Release|xxxxx"。
* 同理，想 build x64 版本，则加参数 `/p:Platform=x64`，且保证 project file 里写了 “xxxxx|x64”
* 小结：在根节点 `<Project>` 下，至少要包括如下节点：

		<ItemGroup>  
			<ProjectConfiguration Include="Debug|Win32" />  
		</ItemGroup> 

* 关于 ProjectConfiguration 更详细信息见本文后面部分。

#### 2. PlatformToolset（Property）
* 必须重定义 PlatformToolset 这个 Property，来指明使用的 VC++ 编译工具链版本。 
* 为什么是‘重定义’？ 因为我们在定义之前需要先 Import Microsoft.Cpp.default.props ，这里面已经定义了 PlatformToolset 以及其他一堆 Property。
* PlatformToolset 默认是 v100（即 vs2010），我们需要用 vs2013，所以重定义成 v120
* 除了 PlatformToolset，还有一些 Property 是经常需要重定义的，比如：ConfigurationType
* ConfigurationType 可以是：Application，StaticLibrary，DynamicLibrary，指定你要 build 的是可执行程序还是库。因为默认就是 Application，所以这里我们不需要重定义。
* 小结：在根节点 `<Project>` 下，至少要包括如下节点，注意，Import 是必须的：

		<Import Project="$(VCTargetsPath)\Microsoft.Cpp.default.props" />  
		<PropertyGroup>  
			<PlatformToolset>v120</PlatformToolset>  
		</PropertyGroup>  

#### 3. 加入源文件 Item 之前，必须先 Import Microsoft.Cpp.props
* 这个没有深究，总之在加入代表源文件的那些 Item 之前，Import 一下是必须的
* 小结：在根节点 `<Project>` 下，在加入代表源文件的那些 Item 之前，必须有如下的节点：

		<Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />  

#### 4. Target 不手写，都是 Import
* VC++ 的 Target 比较复杂，手写不太现实，一般都是 Import 微软为我们准备好的: Microsoft.Cpp.Targets
* 这个 targets 里面不仅包含了默认的的 target： build，还有 clean 等等。我们可以在命令行里用 `/t:` 参数指定运行 clean Target：

		msbuild test.vcxproj /t:clean

* 该命令删除 Debug 目录下生成的目标文件和exe文件，并生成新的log
* 小结：在 project file 的最后， Import targets:

		<Import Project="$(VCTargetsPath)\Microsoft.Cpp.Targets" /> 

### 二。最简单VC++ project file:
* 如下是最简单的，hello world 的 project file，test.vcxproj（在同目录下有hello.c）：

		<Project DefaultTargets="Build" ToolsVersion="12.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">  
			<ItemGroup>  
				<ProjectConfiguration Include="Debug|Win32" />  
			</ItemGroup>  
			<Import Project="$(VCTargetsPath)\Microsoft.Cpp.default.props" />  
			<PropertyGroup>  
				<PlatformToolset>v120</PlatformToolset>  
			</PropertyGroup>  
			<Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />  
			<ItemGroup>  
				<ClCompile Include="hello.c" />  
			</ItemGroup>   
			<Import Project="$(VCTargetsPath)\Microsoft.Cpp.Targets" />  
		</Project>      

* 用如下命令进行测试（需要在 Visual Studio 的命令行）：

		msbuild test.vcxproj

* 命令结束后，会创建 Debug 文件夹，里面有生成的 test.exe

### 三。深入

#### 1. ProjectConfiguration
* ProjectConfiguration 是个 Item，它的值用 Include 属性来赋值。
* 它的值是两部分的组合，用竖线"|"分隔，竖线前面表示 Configuration，竖线后面表示 Platform

##### 1.1 Configuration
* Configuration 的作用主要是供后续的元素作为 Condition 的，其实也就是个标记的作用。
* Configuration 可以随便起名，不一定非得是 Debug，Release，比如自己搞一个 MyConfig 也行
* 举例：

		<ItemGroup>  
			<!-- 运行 Debug 使用参数 /p:Configuration=Debug，或者不加参数，默认选择 Debug --> 
			<!-- 运行 Release 使用参数 /p:Configuration=Release --> 
			<!-- 运行 MyConfig 使用参数 /p:Configuration=MyConfig --> 
			<ProjectConfiguration Include="Debug|Win32" />
			<ProjectConfiguration Include="Release|Win32" />
			<ProjectConfiguration Include="MyConfig|Win32" />
		</ItemGroup> 

##### 1.2 Platform
* Platform 跟 Configuration 类似，不过，Platform 只支持 MSBuild 认识的两种：Win32 和 x64
* 如果你自己定义了一个 Platform，运行 MSBuild 时是会报错的：

		... specified a non-default Platform that doesn't exist for this project.

* 举例：

		<ItemGroup>  
			<!-- 运行 Win32 使用参数 /p:Platform=Win32，或者不加参数，默认选择 Win32 --> 
			<!-- 运行 x64 使用参数 /p:Platform=x64 --> 
			<!-- 运行 MyPlatform 使用参数 /p:Platform=MyPlatform，不过会报错，运行不成功 --> 
			<ProjectConfiguration Include="Debug|Win32" />
			<ProjectConfiguration Include="Debug|x64" />
			<ProjectConfiguration Include="Debug|MyPlatform" />
		</ItemGroup>  