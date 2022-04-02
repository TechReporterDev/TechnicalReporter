@echo off
setlocal enableextensions enabledelayedexpansion

SET MYPATH=%~dp0
call :parent_path DEVPATH %MYPATH:~0,-1%
SET THIRDPARTY=%DEVPATH%3rdParty\

IF "%~1" == "debug" (

	call :install debug

) ELSE IF "%~1" == "release" (

	call :install release

) ELSE IF "%~1" == "" (

	call :install debug
	call :install release

) ELSE (
	echo Error: unknown target
	exit /b 1	
)

goto EOF:


:install <target> (
	SET TARGET=%~1
 
	IF !TARGET! == debug (
		SET OUTPUT=%MYPATH%x64\Debug
		SET VCPKG_BINS=%THIRDPARTY%vcpkg\installed\x64-windows\debug\bin
	

		echo Copying binaries from "vcpkg\...\debug\bin" to "x64\debug"
	
		IF NOT exist !OUTPUT! (
			mkdir !OUTPUT!  || goto :error
		)
 	
		FOR %%f IN (
			boost_filesystem-vc142-mt-gd-x64-1_78.dll
			boost_program_options-vc142-mt-gd-x64-1_78.dll
			boost_regex-vc142-mt-gd-x64-1_78.dll
			boost_serialization-vc142-mt-gd-x64-1_78.dll
			boost_thread-vc142-mt-gd-x64-1_78.dll
			brotlicommon.dll
			brotlidec.dll
			bz2d.dll
			curlpp.dll
			fontconfig-1.dll
			freetyped.dll
			glew32d.dll
			iconv-2.dll
			jpeg62.dll
			libcrypto-1_1-x64.dll
			libcurl-d.dll
			libexslt.dll
			libhpdfd.dll
			libmysql.dll
			libodb.dll
			libodb-boost.dll
			libodb-mysql.dll
			liblzma.dll
			libpng16d.dll
			libxml2.dll
			libxslt.dll
			libssl-1_1-x64.dll
			podofo.dll
			pthreadVC3d.dll
			ssh.dll
			tiffd.dll
			wtd.dll
			wthttpd.dll
			zlibd1.dll) DO (
		xcopy !VCPKG_BINS!\%%f !OUTPUT! /e /y /q > null  || goto :error
		)
	) ELSE IF !TARGET! == release (
		SET OUTPUT=%MYPATH%x64\Release
		SET VCPKG_BINS=%THIRDPARTY%vcpkg\installed\x64-windows\bin
	

		echo Copying binaries from "vcpkg\...\bin" to "x64\release"
	
		IF NOT exist !OUTPUT! (
			mkdir !OUTPUT!  || goto :error
		)
 	
		FOR %%f IN (
			boost_filesystem-vc142-mt-x64-1_78.dll
			boost_program_options-vc142-mt-x64-1_78.dll
			boost_regex-vc142-mt-x64-1_78.dll
			boost_serialization-vc142-mt-x64-1_78.dll
			boost_thread-vc142-mt-x64-1_78.dll
			brotlicommon.dll
			brotlidec.dll
			bz2.dll
			curlpp.dll
			fontconfig-1.dll
			freetype.dll
			glew32.dll
			iconv-2.dll
			jpeg62.dll
			libcrypto-1_1-x64.dll
			libcurl.dll
			libexslt.dll
			libhpdf.dll
			libmysql.dll
			libodb.dll
			libodb-boost.dll
			libodb-mysql.dll
			liblzma.dll
			libpng16.dll
			libxml2.dll
			libxslt.dll
			libssl-1_1-x64.dll
			podofo.dll
			pthreadVC3.dll
			ssh.dll
			tiff.dll
			wt.dll
			wthttp.dll
			zlib1.dll) DO (
		xcopy !VCPKG_BINS!\%%f !OUTPUT! /e /y /q > null  || goto :error
		)
	) ELSE (
		echo Error: unknown target
		exit /b 1
	)

	SET DOCROOT=!OUTPUT!\docroot
	SET DOCROOT_RESOURCES=!DOCROOT!\resources
	SET WT_RESOURCES=%THIRDPARTY%vcpkg\installed\x64-windows\share\wt\resources
 
	echo Copying resources from "vcpkg\...\share\wt\resources" to "x64\!TARGET!\docroot"

	IF NOT exist !DOCROOT_RESOURCES! (
		mkdir !DOCROOT_RESOURCES!  || goto :error
	)

	xcopy !WT_RESOURCES! !DOCROOT_RESOURCES! /e /y /q > null  || goto :error

	SET SERVER_RESOURCES=%MYPATH%TRWebServer\docroot 
	echo Copying resources from "TRWebServer\docroot" to "x64\!TARGET!\docroot"

	xcopy !SERVER_RESOURCES! !DOCROOT! /e /y /q > null  || goto :error

	SET FEATURES=%MYPATH%Features
	SET OUTPUT_FEATURES=!OUTPUT!\Features
	echo Copying resources from "Features" to "x64\!TARGET!\Features"

	IF NOT exist !OUTPUT_FEATURES! (
		mkdir !OUTPUT_FEATURES!  || goto :error
	)

	xcopy !FEATURES! !OUTPUT_FEATURES! /e /y /q > null  || goto :error
	exit /b
)

:parent_path <result> <path>
(
	set "%~1=%~dp2"
	exit /b
)

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%

:EOF
echo Installation completed