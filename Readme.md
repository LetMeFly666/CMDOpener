<!--
 * @Author: LetMeFly
 * @Date: 2022-03-13 14:22:08
 * @LastEditors: LetMeFly
 * @LastEditTime: 2022-03-13 19:16:34
-->
# CMDOpener

通过<code>Win+R</code>的方式打开一些文件夹

## 使用效果

<img src="Demo.gif">

## 使用方式

运行CMDOpener.exe，程序自动监测环境变量中是否已经存在TFpath

+ 若不存在(首次使用)则会产生一个文件夹并加入环境变量

+ 若已经存在(再次使用)则会让用户选择一个文件夹来当作快速打开用的文件夹，并让用户命名

## 特殊修改

修改环境变量<code>TFpath</code>的值可改变脚本存放路径。

但是脚本都是字节级别的文件，不会占据C盘太大的空间。

## 备选方案

若实在不行可以在任务管理器中找到“Windows 资源管理器”，右键“重新启动”
