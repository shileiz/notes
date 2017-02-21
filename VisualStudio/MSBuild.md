* 参考1：[https://msdn.microsoft.com/en-us/library/dd393573.aspx](https://msdn.microsoft.com/en-us/library/dd393573.aspx)

##开始之前
* 本文参考的 MSDN 文档描述的 MSBuild 是基于 C#/VB 的，而 VC++ 略有不同
* 本文的基本概念对于 VC++ 是一样的，请放心阅读
* 关于 VC++ 的 MSBuild，可以参考：
	* [MSBuild (Visual C++) Overview](https://msdn.microsoft.com/en-us/library/ee662426.aspx)
	* [MSBuild (Visual C++)](https://msdn.microsoft.com/en-us/library/dd293626.aspx)
	* [MSBuild Tasks Specific to Visual C++](https://msdn.microsoft.com/en-us/library/ff960151.aspx)
	* [CL Task](https://msdn.microsoft.com/en-us/library/ee862477.aspx)


##基本概念

### 1。Project File
* MSBuild is the build platform for Microsoft and Visual Studio. 
* MSBuild 根据 project file 工作，project file 是标准的 xml 文件，后缀名是 .vcxproj(对于C++), .vbproj(对于VB)，.csproj(对于C#)
* 可以在 Visual Studio 命令行提示符里运行一个 project file：

		msbuild aaaaa.vcxproj

* project file 的根节点必须是 Project：

		<?xml version="1.0" encoding="utf-8"?>  
		<Project ToolsVersion="12.0" DefaultTargets="Build"  xmlns="http://schemas.microsoft.com/developer/msbuild/2003">  


### 2。Targets and Tasks
* project file 用 Target 和 Task 来告诉 MSBuild 如何完成编译工作
* Task 是 MSBuild 的最小执行单元，Target 是一系列 Task 的集合
* Task 不能在 project file 里定义，需要用.NET写好，在 project file 里使用。
* 使用方法是： 在 Target 标签下加入一个以 task 为名的标签，比如：

		<Target Name="MakeBuildDirectory">  
		    <MakeDir  
		        Directories="$(BuildDir)" />  
		</Target>  

* MakeDir 就是一个 Task，是用 .net 实现的。Directories="$(BuildDir)" 可以理解为传给他的“参数”
* 根节点 Project 的 DefaultTargets 属性指定默认的 build target，这个属性是可选的。

		<Project ToolsVersion="12.0" DefaultTargets="Build" ...  

* 也可以在运行 MSBuild 的时候用命令行参数 /t 指定 Target：

		msbuild buildapp.vcxproj /t:HelloWorld  

* Target 一般不是在 project file 里定义，而是 import 进来：

		<Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />

### 3。Properties

#### 定义 Property
* PropertyGroup 的子节点就是定一个 Property，例如：

		<PropertyGroup>  
			<TargetFrameworkVersion>v12.0</TargetFrameworkVersion>
		</PropertyGroup>  

* 定义了一个名为 TargetFrameworkVersion 的 Property，其值为 v12.0
* 后定义的同名 Property 会 override 先定义的。

#### 引用 Property 的值
* 用如下语法引用一个 Property 的值，比如引用上述定义的 TargetFrameworkVersion：

		$(TargetFrameworkVersion) 

* 未定义的 Property 引用结果将是空字串，即 ''

#### 保留的 Property
* MSBuild 有一些保留的 Property,不定义也能引用到，比如：MSBuildToolsPath
* 根据 [这里](https://msdn.microsoft.com/en-us/library/ms164309.aspx) 的表格，常用的 保留 Property 都是以 MSBuild 开头的

#### 条件定义 Property
* 在定义的 Property 节点使用 Condition 属性可以根据条件是否满足来定义 Property

		<Configuration   Condition=" '$(Configuration)' == '' ">Debug</Configuration>

* 以上例子表示仅当 Configuration 未定义的时候，才定义 Configuration 为 Debug
* 不光 Property 节点可以使用 Condition 属性，几乎 project file 里的所有节点都可以使用 Condition 属性 


### 4。Items  

#### 定义 Item
* Item 是 ItemGroup 的子节点，节点名就是 Item 的名，节点的 Include 属性指定了 Item 的值。
* 同名的多个 Item 的值构成了一类 Item，即 ItemType

		<ItemGroup>  
		    <Compile Include="main.cpp" />  
		    <Compile Include="calc\add.cpp" />  
		</ItemGroup>  

* 定义了两个叫 Compile 的 Item， 他们的值分别是 main.cpp 和 calc\add.cpp
* 同时我们有了一个叫 Compile 的 ItemType

#### 引用 Item 的值
* 用如下语法引用一个 ItemType 的所有值，比如上述定义的 Compile：

		@(Compile) 

* 将引用到 main.cpp;calc\add.cpp
* 分号是默认的分隔符，也可以自定义，但一般用不上。

#### Item 的作用
* Defines the inputs for the build process. [参考这里](https://msdn.microsoft.com/en-us/library/646dk05y.aspx)

#### Item Metadata
* 每一个 Item 都有默认的 Metadata（Well-known Item Metadata），比如 Filename
* 引用某个 ItemType 的 Metdata 的语法是：

		%(ItemType.MetaDataName)  

* 比如：

		%(Compile.Filename) 

##### 默认 Metadata 
* [Well-known Item Metadata](https://msdn.microsoft.com/en-us/library/ms164313.aspx) 即默认的 Metadata，每个 Item 自带这些 Metadata
* 共有如下这些 ：
		
		FullPath,RootDir,Filename,Extension,RelativeDir,Directory,RecursiveDir,Identity,ModifiedTime,CreatedTime,AccessedTime

##### 自定义 Metadata
* 给一个 Item 定义 Metadata 的语法是，在 Item 节点下添加子节点。例如：

		<ItemGroup>  
		    <CSFile Include="main.cs">  
		        <Culture>Fr</Culture>  
		    </CSFile>  
		</ItemGroup>  

* 以上给 CSFile 这个 Item 定义了一个叫 Culture 的 Metadata，这个 Metadata 的值是 Fr。
* 同样可以引用这个 Metadata 的值：

		%(CSFile.Culture) 

##### 自定义默认 Metadata
* 可以用 [ItemDefinitionGroup](https://msdn.microsoft.com/en-us/library/bb629392.aspx) 给某类 Item 设置默认的 Metadata：

		<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">  
		    <ItemDefinitionGroup>  
		        <i>  
		            <m>m1</m>  
		            <n>n1</n>  
		        </i>        
		    </ItemDefinitionGroup>  
		    <ItemGroup>  
		        <i Include="a">  
		            <o>o1</o>  
		            <n>n2</n>  
		        </i>  
		    </ItemGroup>  
		    ...  
		</Project>  

* 以上代码给 Item i，设置了 m 和 n 两个默认 Metadata
* 但定义 Item i 的时候，默认 Metadata n 被覆盖了，所以 i.n 等于 n2，而 m 没有被覆盖，所以 i.m 等于 m1

### 5。一个测试样例
* 新建一个 test.csproj，内容如下：

		<?xml version="1.0" encoding="utf-8"?>  
		<Project ToolsVersion="12.0"  xmlns="http://schemas.microsoft.com/developer/msbuild/2003">  
		
			<ItemGroup>  
				<Compile Include="main.cpp" >
					<meta1>m1</meta1>
				</Compile>
				<Compile Include="calc\add.cpp" > 
					<meta1>m2</meta1>	
				</Compile>
			</ItemGroup>  
		
		
			<Target Name="HelloWorld">  
			  <Message Text="%(Compile.Filename)"></Message>
			  <Message Text="---------------"></Message>  
			  <Message Text="@(Compile)"></Message>
			  <Message Text="---------------"></Message>  
			  <Message Text="%(Compile.meta1)"></Message>
			</Target>  
			
		</Project>

* 运行如下命令来测试：

		msbuild test.csproj /t:HelloWorld



