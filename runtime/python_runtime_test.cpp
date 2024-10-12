#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define NOMINMAX
#include "cdts_python3.h"
#include "py_bytes.h"
#include "py_float.h"
#include "py_list.h"
#include "py_tuple.h"
#include "runtime_id.h"
#include "utils.h"
#include <doctest/doctest.h>
#include <filesystem>
#include <runtime/runtime_plugin_api.h>
#include <runtime/xcall.h>

#ifdef _DEBUG
#undef _DEBUG

#include <Python.h>

#define _DEBUG
#else
#include <Python.h>
#endif

struct GlobalSetup {
	std::string runtime_test_target_path;

	GlobalSetup()
	{
		if(std::getenv("METAFFI_HOME") == nullptr)
		{
			std::cerr << "METAFFI_HOME is not set. Please set it to the root of the Metaffi project" << std::endl;
			exit(1);
		}

		std::filesystem::path runtime_test_target_std_path(__FILE__);
		runtime_test_target_std_path = runtime_test_target_std_path.parent_path();
		runtime_test_target_std_path.append("./test/runtime_test_target.py");
		runtime_test_target_path = runtime_test_target_std_path.string();

		char* err = nullptr;
		uint32_t err_len = 0;
		load_runtime(&err);

		if(err)
		{
			std::cerr << "load runtime failed with " << err << std::endl;
			exit(2);
		}

		if(err_len != 0)
		{
			std::cerr << "Error length is not 0. Expected to be 0" << std::endl;
			exit(3);
		}
	}

	~GlobalSetup()
	{
		char* err = nullptr;
		free_runtime(&err);

		if(err)
		{
			std::cerr << "free runtime failed with " << err << std::endl;
			exit(4);
		}
	}
};

static GlobalSetup setup;

char* err = nullptr;

TEST_SUITE("CDTS Python3.11 Test")
{

	TEST_CASE("Traverse 2D bytes array")
	{
		pyscope();

		cdts* pcdts = xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg([&pcdts]() { xllr_free_cdts_buffer(pcdts); });
		
		pcdts->arr[0].set_array(new cdts(3, 2), metaffi_uint8_type);
		cdt& cdt_array2d = pcdts->arr[0];
		(*cdt_array2d.cdt_val.array_val)[0].set_new_array(2, 1, metaffi_uint8_type);
		(*cdt_array2d.cdt_val.array_val)[1].set_new_array(3, 1, metaffi_uint8_type);
		(*cdt_array2d.cdt_val.array_val)[2].set_new_array(4, 1, metaffi_uint8_type);

		(*cdt_array2d.cdt_val.array_val->arr[0].cdt_val.array_val)[0] = (metaffi_uint8)1;
		(*cdt_array2d.cdt_val.array_val->arr[0].cdt_val.array_val)[1] = (metaffi_uint8)2;

		(*cdt_array2d.cdt_val.array_val->arr[1].cdt_val.array_val)[0] = (metaffi_uint8)3;
		(*cdt_array2d.cdt_val.array_val->arr[1].cdt_val.array_val)[1] = (metaffi_uint8)4;
		(*cdt_array2d.cdt_val.array_val->arr[1].cdt_val.array_val)[2] = (metaffi_uint8)5;

		(*cdt_array2d.cdt_val.array_val->arr[2].cdt_val.array_val)[0] = (metaffi_uint8)6;
		(*cdt_array2d.cdt_val.array_val->arr[2].cdt_val.array_val)[1] = (metaffi_uint8)7;
		(*cdt_array2d.cdt_val.array_val->arr[2].cdt_val.array_val)[2] = (metaffi_uint8)8;
		(*cdt_array2d.cdt_val.array_val->arr[2].cdt_val.array_val)[3] = (metaffi_uint8)9;

		cdts_python3 cdts_py(*pcdts);

		py_tuple t = cdts_py.to_py_tuple();
		py_list lst(t[0]);

		REQUIRE(((lst.length() == 3)));

		py_bytes b0(lst[0]);
		py_bytes b1(lst[1]);
		py_bytes b2(lst[2]);

		REQUIRE(((b0.size() == 2)));
		REQUIRE(((b0[0] == 1)));
		REQUIRE(((b0[1] == 2)));

		REQUIRE(((b1.size() == 3)));
		REQUIRE(((b1[0] == 3)));
		REQUIRE(((b1[1] == 4)));

		REQUIRE(((b2.size() == 4)));
		REQUIRE(((b2[0] == 6)));
		REQUIRE(((b2[1] == 7)));
		REQUIRE(((b2[2] == 8)));
		REQUIRE(((b2[3] == 9)));
	}


	TEST_CASE("Traverse 1D bytes array")
	{
		pyscope();

		cdts* pcdts = xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg([&pcdts]() { xllr_free_cdts_buffer(pcdts); });
		pcdts->arr[0].set_new_array(3, 1, metaffi_uint8_type);
		cdt& cdt_array1d = pcdts->arr[0];
		cdt_array1d.cdt_val.array_val->arr[0].cdt_val.uint8_val = 1;
		cdt_array1d.cdt_val.array_val->arr[1].cdt_val.uint8_val = 2;
		cdt_array1d.cdt_val.array_val->arr[2].cdt_val.uint8_val = 3;

		cdts_python3 cdts_py(*pcdts);
		py_tuple t = cdts_py.to_py_tuple();
		py_bytes bytes(t[0]);
		REQUIRE(((bytes.size() == 3)));

		py_int b1(bytes[0]);
		py_int b2(bytes[1]);
		py_int b3(bytes[2]);

		REQUIRE((((metaffi_uint8)b1 == 1)));
		REQUIRE((((metaffi_uint8)b2 == 2)));
		REQUIRE((((metaffi_uint8)b3 == 3)));
	}


	TEST_CASE("Traverse 1D array")
	{
		pyscope();

		cdts* pcdts = xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg([&pcdts]() { xllr_free_cdts_buffer(pcdts); });
		pcdts->arr[0].set_new_array(3, 1, metaffi_float32_type);
		cdt& cdt_array1d = pcdts->arr[0];
		cdt_array1d.cdt_val.array_val->arr[0] = (metaffi_float32)1.0f;
		cdt_array1d.cdt_val.array_val->arr[1] = (metaffi_float32)2.0f;
		cdt_array1d.cdt_val.array_val->arr[2] = (metaffi_float32)3.0f;

		cdts_python3 cdts_py(*pcdts);
		py_tuple t = cdts_py.to_py_tuple();
		py_list lst(t[0]);
		REQUIRE(((lst.length() == 3)));

		py_float f1(lst[0]);
		py_float f2(lst[1]);
		py_float f3(lst[2]);

		REQUIRE((((metaffi_float32)f1 == 1.0f)));
		REQUIRE((((metaffi_float32)f2 == 2.0f)));
		REQUIRE(((metaffi_float32)f3 == 3.0f));
	}

	TEST_CASE("Traverse 3D array")
	{
		pyscope();

		cdts* pcdts = xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg([&pcdts]() { xllr_free_cdts_buffer(pcdts); });
		pcdts->arr[0].set_new_array(2, 3, metaffi_float32_type);
		cdt& cdt_array3d = pcdts->arr[0];
		cdt_array3d.cdt_val.array_val->arr[0].set_new_array(2, 2, metaffi_float32_type);
		cdt_array3d.cdt_val.array_val->arr[1].set_new_array(3, 2, metaffi_float32_type);

		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].set_new_array(4, 1, metaffi_float32_type);
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].set_new_array(2, 1, metaffi_float32_type);

		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].set_new_array(3, 1, metaffi_float32_type);
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].set_new_array(2, 1, metaffi_float32_type);
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].set_new_array(1, 1, metaffi_float32_type);

		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = (metaffi_float32)1.0f;
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = (metaffi_float32)2.0f;
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2] = (metaffi_float32)3.0f;
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[3] = (metaffi_float32)4.0f;

		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = (metaffi_float32)5.0f;
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = (metaffi_float32)6.0f;

		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = (metaffi_float32)7.0f;
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = (metaffi_float32)8.0f;
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2] = (metaffi_float32)9.0f;

		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = (metaffi_float32)10.0f;
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = (metaffi_float32)11.0f;

		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].cdt_val.array_val->arr[0] = (metaffi_float32)12.0f;

		cdts_python3 cdts_py(*pcdts);
		py_tuple t = cdts_py.to_py_tuple();
		py_list lst3dim(t[0]);

		REQUIRE((lst3dim.length() == 2));

		py_list lst2dim0(lst3dim[0]);
		py_list lst2dim1(lst3dim[1]);

		REQUIRE((lst2dim0.length() == 2));
		REQUIRE((lst2dim1.length() == 3));

		py_list lst1dim00(lst2dim0[0]);
		py_list lst1dim01(lst2dim0[1]);

		py_list lst1dim10(lst2dim1[0]);
		py_list lst1dim11(lst2dim1[1]);
		py_list lst1dim12(lst2dim1[2]);

		REQUIRE((lst1dim00.length() == 4));
		REQUIRE((lst1dim01.length() == 2));
		REQUIRE((lst1dim10.length() == 3));
		REQUIRE((lst1dim11.length() == 2));
		REQUIRE((lst1dim12.length() == 1));

		REQUIRE(((metaffi_float32)py_float(lst1dim00[0]) == 1.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim00[1]) == 2.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim00[2]) == 3.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim00[3]) == 4.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim01[0]) == 5.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim01[1]) == 6.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim10[0]) == 7.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim10[1]) == 8.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim10[2]) == 9.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim11[0]) == 10.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim11[1]) == 11.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim12[0]) == 12.0f));
	}

	TEST_CASE("Construct 1D bytes array")
	{
		pyscope();

		cdts* pcdts = xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg([&pcdts]() { xllr_free_cdts_buffer(pcdts); });
		pcdts->arr[0].set_new_array(3, 1, metaffi_uint8_type);
		cdt& cdt_array1d = pcdts->arr[0];
		cdt_array1d.cdt_val.array_val->arr[0] = (metaffi_uint8)1;
		cdt_array1d.cdt_val.array_val->arr[1] = (metaffi_uint8)2;
		cdt_array1d.cdt_val.array_val->arr[2] = (metaffi_uint8)3;

		cdts_python3 cdts_py(*pcdts);
		py_tuple t = cdts_py.to_py_tuple();
		py_bytes bytes(t[0]);
		REQUIRE((bytes.size() == 3));

		py_int b1(bytes[0]);
		py_int b2(bytes[1]);
		py_int b3(bytes[2]);

		REQUIRE(((metaffi_uint8)b1 == 1));
		REQUIRE(((metaffi_uint8)b2 == 2));
		REQUIRE(((metaffi_uint8)b3 == 3));
	}

	TEST_CASE("Construct 2D bytes array")
	{
		pyscope();

		cdts* pcdts = xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg([&pcdts]() { xllr_free_cdts_buffer(pcdts); });
		pcdts->arr[0].set_new_array(2, 2, metaffi_uint8_type);
		cdt& cdt_array2d = pcdts->arr[0];
		cdt_array2d.cdt_val.array_val->arr[0].set_new_array(3, 1, metaffi_uint8_type);
		cdt_array2d.cdt_val.array_val->arr[1].set_new_array(3, 1, metaffi_uint8_type);

		cdt_array2d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = ((metaffi_uint8)1);
		cdt_array2d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = ((metaffi_uint8)2);
		cdt_array2d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[2] = ((metaffi_uint8)3);

		cdt_array2d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = ((metaffi_uint8)4);
		cdt_array2d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = ((metaffi_uint8)5);
		cdt_array2d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[2] = ((metaffi_uint8)6);

		cdts_python3 cdts_py(*pcdts);
		py_tuple t = cdts_py.to_py_tuple();
		py_list lst(t[0]);
		REQUIRE((lst.length() == 2));

		py_bytes bytes1(lst[0]);
		py_bytes bytes2(lst[1]);

		REQUIRE((bytes1.size() == 3));
		REQUIRE((bytes2.size() == 3));

		py_int b11(bytes1[0]);
		py_int b12(bytes1[1]);
		py_int b13(bytes1[2]);

		py_int b21(bytes2[0]);
		py_int b22(bytes2[1]);
		py_int b23(bytes2[2]);

		REQUIRE(((metaffi_uint8)b11 == 1));
		REQUIRE(((metaffi_uint8)b12 == 2));
		REQUIRE(((metaffi_uint8)b13 == 3));

		REQUIRE(((metaffi_uint8)b21 == 4));
		REQUIRE(((metaffi_uint8)b22 == 5));
		REQUIRE(((metaffi_uint8)b23 == 6));
	}

	TEST_CASE("Construct 1D array")
	{
		pyscope();

		cdts* pcdts = xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg([&pcdts]() { xllr_free_cdts_buffer(pcdts); });
		pcdts->arr[0].set_new_array(3, 1, metaffi_float32_type);
		cdt& cdt_array1d = pcdts->arr[0];
		cdt_array1d.cdt_val.array_val->arr[0] = ((metaffi_float32)1.0f);
		cdt_array1d.cdt_val.array_val->arr[1] = ((metaffi_float32)2.0f);
		cdt_array1d.cdt_val.array_val->arr[2] = ((metaffi_float32)3.0f);

		cdts_python3 cdts_py(*pcdts);
		py_tuple t = cdts_py.to_py_tuple();
		py_list lst(t[0]);
		REQUIRE((lst.length() == 3));

		py_float f1(lst[0]);
		py_float f2(lst[1]);
		py_float f3(lst[2]);

		REQUIRE(((metaffi_float32)f1 == 1.0f));
		REQUIRE(((metaffi_float32)f2 == 2.0f));
		REQUIRE(((metaffi_float32)f3 == 3.0f));
	}

	TEST_CASE("Construct 3D array")
	{
		pyscope();

		cdts* pcdts = xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg([&pcdts]() { xllr_free_cdts_buffer(pcdts); });
		pcdts->arr[0].set_new_array(2, 3, metaffi_float32_type);
		cdt& cdt_array3d = pcdts->arr[0];
		cdt_array3d.cdt_val.array_val->arr[0].set_new_array(2, 2, metaffi_float32_type);
		cdt_array3d.cdt_val.array_val->arr[1].set_new_array(3, 2, metaffi_float32_type);

		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].set_new_array(4, 1, metaffi_float32_type);
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].set_new_array(2, 1, metaffi_float32_type);

		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].set_new_array(3, 1, metaffi_float32_type);
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].set_new_array(2, 1, metaffi_float32_type);
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].set_new_array(1, 1, metaffi_float32_type);

		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = ((metaffi_float32)1.0f);
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = ((metaffi_float32)2.0f);
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2] = ((metaffi_float32)3.0f);
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[3] = ((metaffi_float32)4.0f);

		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = ((metaffi_float32)5.0f);
		cdt_array3d.cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = ((metaffi_float32)6.0f);

		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = ((metaffi_float32)7.0f);
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = ((metaffi_float32)8.0f);
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2] = ((metaffi_float32)9.0f);

		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = ((metaffi_float32)10.0f);
		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = ((metaffi_float32)11.0f);

		cdt_array3d.cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].cdt_val.array_val->arr[0] = ((metaffi_float32)12.0f);

		cdts_python3 cdts_py(*pcdts);
		py_tuple t = cdts_py.to_py_tuple();
		py_list lst3dim(t[0]);

		REQUIRE((lst3dim.length() == 2));

		py_list lst2dim0(lst3dim[0]);
		py_list lst2dim1(lst3dim[1]);

		REQUIRE((lst2dim0.length() == 2));
		REQUIRE((lst2dim1.length() == 3));

		py_list lst1dim00(lst2dim0[0]);
		py_list lst1dim01(lst2dim0[1]);

		py_list lst1dim10(lst2dim1[0]);
		py_list lst1dim11(lst2dim1[1]);
		py_list lst1dim12(lst2dim1[2]);

		REQUIRE((lst1dim00.length() == 4));
		REQUIRE((lst1dim01.length() == 2));
		REQUIRE((lst1dim10.length() == 3));
		REQUIRE((lst1dim11.length() == 2));
		REQUIRE((lst1dim12.length() == 1));

		REQUIRE(((metaffi_float32)py_float(lst1dim00[0]) == 1.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim00[1]) == 2.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim00[2]) == 3.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim00[3]) == 4.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim01[0]) == 5.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim01[1]) == 6.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim10[0]) == 7.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim10[1]) == 8.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim10[2]) == 9.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim11[0]) == 10.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim11[1]) == 11.0f));
		REQUIRE(((metaffi_float32)py_float(lst1dim12[0]) == 12.0f));
	}
}

xcall* cpp_load_entity(const std::string& module_path,
                        const std::string& entity_path,
                        std::vector<metaffi_type_info> params_types,
                        std::vector<metaffi_type_info> retvals_types)
{
	err = nullptr;

	metaffi_type_info* params_types_arr = params_types.empty() ? nullptr : params_types.data();
	metaffi_type_info* retvals_types_arr = retvals_types.empty() ? nullptr : retvals_types.data();

	xcall* pfunction = load_entity(module_path.c_str(),
	                               entity_path.c_str(),
	                               params_types_arr, (int8_t)params_types.size(),
	                               retvals_types_arr, (int8_t)retvals_types.size(),
	                               &err);

	if(err)
	{
		FAIL(std::string(err));
	}
	REQUIRE((pfunction->pxcall_and_context[0] != nullptr));
	REQUIRE((pfunction->pxcall_and_context[1] != nullptr));

	return pfunction;
};


TEST_SUITE("Python Runtime Tests")
{

	TEST_CASE("runtime_test_target.hello_world")
	{
		std::string entity_path = "callable=hello_world";
		xcall* phello_world = cpp_load_entity(setup.runtime_test_target_path, entity_path, {}, {});
		metaffi::utils::scope_guard sg([&] {
			free_xcall(phello_world, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		(*phello_world)(&err);
		if(err)
		{
			FAIL(std::string(err));
		}
	}

	TEST_CASE("runtime_test_target.returns_an_error")
	{
		std::string entity_path = "callable=returns_an_error";
		xcall* preturns_an_error = cpp_load_entity(setup.runtime_test_target_path, entity_path, {}, {});
		metaffi::utils::scope_guard sg([&] {
			free_xcall(preturns_an_error, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		(*preturns_an_error)(&err);
		REQUIRE((err != nullptr));
		free(err);

		err = nullptr;
	}

	TEST_CASE("runtime_test_target.div_integers")
	{
		std::string entity_path = "callable=div_integers";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_int64_type},
		                                               metaffi_type_info{metaffi_int64_type}};
		std::vector<metaffi_type_info> retvals_types = {metaffi_type_info{metaffi_float64_type}};

		xcall* pdiv_integers = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_types, retvals_types);
		metaffi::utils::scope_guard sg([&] {
			free_xcall(pdiv_integers, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), retvals_types.size());
		metaffi::utils::scope_guard sg2([&] { xllr_free_cdts_buffer(cdts_param_ret); });
		cdts& params = cdts_param_ret[0];
		cdts& ret = cdts_param_ret[1];

		params[0] = ((metaffi_int64)10);
		params[1] = ((metaffi_int64)2);

		(*pdiv_integers)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		REQUIRE((ret[0].type == metaffi_float64_type));
		REQUIRE((ret[0].cdt_val.float64_val == 5.0));
	}

	TEST_CASE("runtime_test_target.join_strings")
	{
		std::string entity_path = "callable=join_strings";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_string8_array_type}};
		std::vector<metaffi_type_info> retvals_types = {metaffi_type_info{metaffi_string8_type}};

		xcall* join_strings = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_types,
		                                      retvals_types);
		metaffi::utils::scope_guard sg([&] {
			free_xcall(join_strings, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), retvals_types.size());
		metaffi::utils::scope_guard sg2([&] { xllr_free_cdts_buffer(cdts_param_ret); });

		cdts& params = cdts_param_ret[0];
		cdts& ret = cdts_param_ret[1];

		params[0].set_new_array(3, 1, metaffi_string8_type);
		params[0].cdt_val.array_val->arr[0].set_string((metaffi_string8) "one", true);
		params[0].cdt_val.array_val->arr[1].set_string((metaffi_string8) "two", true);
		params[0].cdt_val.array_val->arr[2].set_string((metaffi_string8) "three", true);

		(*join_strings)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		REQUIRE((ret[0].type == metaffi_string8_type));
		REQUIRE((std::u8string(ret[0].cdt_val.string8_val) == u8"one,two,three"));
	}

	TEST_CASE("runtime_test_target.wait_a_bit")
	{
		// get five_seconds global
		std::vector<metaffi_type_info> var_type = {metaffi_type_info{metaffi_int64_type}};
		std::string variable_path = "attribute=five_seconds,getter";
		xcall* pfive_seconds_getter = cpp_load_entity(setup.runtime_test_target_path, variable_path, {}, var_type);
		metaffi::utils::scope_guard sg3([&] {
			free_xcall(pfive_seconds_getter, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});
		
		cdts* getter_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg([&] { xllr_free_cdts_buffer(getter_ret); });
		cdts& ret = getter_ret[1];

		(*pfive_seconds_getter)((cdts*)getter_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
		
		REQUIRE((ret[0].type == metaffi_int64_type));
		REQUIRE((ret[0].cdt_val.int64_val == 5));

		int64_t five = ret[0].cdt_val.int64_val;

		// call wait_a_bit
		std::string entity_path = "callable=wait_a_bit";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_int64_type}};

		xcall* pwait_a_bit = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_types, {});
		metaffi::utils::scope_guard sg1([&] {
			free_xcall(pwait_a_bit, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), 0);
		metaffi::utils::scope_guard sg2([&] { xllr_free_cdts_buffer(cdts_param_ret); });
		cdts& params = cdts_param_ret[0];

		params[0] = ((metaffi_int64)five);

		(*pwait_a_bit)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
	}

	TEST_CASE("runtime_test_target.testmap.set_get_contains")
	{
		// create new testmap
		std::string entity_path = "callable=testmap";
		std::vector<metaffi_type_info> retvals_types = {metaffi_type_info(metaffi_handle_type)};

		xcall* pnew_testmap = cpp_load_entity(setup.runtime_test_target_path, entity_path, {}, retvals_types);
		metaffi::utils::scope_guard sg5([&] {
			free_xcall(pnew_testmap, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});
		
		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg([&] { xllr_free_cdts_buffer(cdts_param_ret); });
		cdts& ret = cdts_param_ret[1];

		(*pnew_testmap)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		REQUIRE((ret[0].type == metaffi_handle_type));
		REQUIRE((ret[0].cdt_val.handle_val->handle != nullptr));
		REQUIRE((ret[0].cdt_val.handle_val->runtime_id == PYTHON312_RUNTIME_ID));

		cdt_metaffi_handle* testmap_instance = ret[0].cdt_val.handle_val;

		// set
		entity_path = "callable=testmap.set,instance_required";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info(metaffi_handle_type),
		                                               metaffi_type_info(metaffi_string8_type),
		                                               metaffi_type_info(metaffi_any_type)};

		xcall* p_testmap_set = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_types, {});
		metaffi::utils::scope_guard sg6([&] {
			free_xcall(p_testmap_set, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		cdts* cdts_param_ret4 = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), 0);
		metaffi::utils::scope_guard sg2([&] { xllr_free_cdts_buffer(cdts_param_ret4); });
		cdts& params = cdts_param_ret4[0];

		params[0].set_handle(testmap_instance);
		params[1].set_string((metaffi_string8)u8"key", true);
		params[2] = (metaffi_int64)42;

		(*p_testmap_set)((cdts*)cdts_param_ret4, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		// contains
		entity_path = "callable=testmap.contains,instance_required";
		params_types = {metaffi_type_info(metaffi_handle_type),
		                metaffi_type_info(metaffi_string8_type)};
		retvals_types = {metaffi_type_info(metaffi_bool_type)};

		xcall* p_testmap_contains = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_types, retvals_types);
		metaffi::utils::scope_guard sg7([&] {
			free_xcall(p_testmap_contains, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		cdts* cdts_param_ret2 = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), retvals_types.size());
		metaffi::utils::scope_guard sg3([&] { xllr_free_cdts_buffer(cdts_param_ret2); });
		cdts& contains_params = cdts_param_ret2[0];
		cdts& contains_ret = cdts_param_ret2[1];

		contains_params[0].set_handle(testmap_instance);
		contains_params[1].set_string((metaffi_string8)u8"key", true);

		(*p_testmap_contains)((cdts*)cdts_param_ret2, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		REQUIRE((contains_ret[0].type == metaffi_bool_type));
		REQUIRE((contains_ret[0].cdt_val.bool_val != 0));

		// get
		entity_path = "callable=testmap.get,instance_required";
		params_types = {metaffi_type_info(metaffi_handle_type),
		                metaffi_type_info(metaffi_string8_type)};
		retvals_types = {metaffi_type_info(metaffi_any_type)};

		xcall* p_testmap_get = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_types, retvals_types);
		metaffi::utils::scope_guard sg8([&] {
			free_xcall(p_testmap_get, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		cdts* cdts_param_ret3 = (cdts*)xllr_alloc_cdts_buffer(params_types.size(), retvals_types.size());
		metaffi::utils::scope_guard sg4([&] { xllr_free_cdts_buffer(cdts_param_ret3); });
		cdts& get_params = cdts_param_ret3[0];
		cdts& get_ret = cdts_param_ret3[1];

		get_params[0].set_handle(testmap_instance);
		get_params[1].set_string((metaffi_string8)u8"key", true);

		(*p_testmap_get)((cdts*)cdts_param_ret3, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
		REQUIRE((get_ret[0].type == metaffi_int64_type));
		REQUIRE((get_ret[0].cdt_val.int64_val == 42));
	}

	TEST_CASE("runtime_test_target.testmap.set_get_contains_cpp_object")
	{
		// create new testmap
		std::string entity_path = "callable=testmap";
		std::vector<metaffi_type_info> retvals_types = {metaffi_type_info(metaffi_handle_type)};

		xcall* pnew_testmap = cpp_load_entity(setup.runtime_test_target_path, entity_path, {}, retvals_types);
		metaffi::utils::scope_guard sg5([&] {
			free_xcall(pnew_testmap, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg([&] { xllr_free_cdts_buffer(cdts_param_ret); });

		(*pnew_testmap)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
		
		cdts& wrapper_ret = cdts_param_ret[1];
		REQUIRE((wrapper_ret[0].type == metaffi_handle_type));
		REQUIRE((wrapper_ret[0].cdt_val.handle_val->handle != nullptr));
		REQUIRE((wrapper_ret[0].cdt_val.handle_val->runtime_id == PYTHON312_RUNTIME_ID));
		REQUIRE((wrapper_ret[0].cdt_val.handle_val->release != nullptr));

		cdt_metaffi_handle* testmap_instance = wrapper_ret[0].cdt_val.handle_val;

		// set
		entity_path = "callable=testmap.set,instance_required";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info(metaffi_handle_type),
		                                               metaffi_type_info(metaffi_string8_type),
		                                               metaffi_type_info(metaffi_any_type)};

		xcall* p_testmap_set = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_types, {});
		metaffi::utils::scope_guard sg6([&] {
			free_xcall(p_testmap_set, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		auto insert = [](
              xcall* p_testmap_set, const cdt_metaffi_handle* hthis, const std::u8string& key,
			std::vector<int8_t>* val,
			int runtime_id) {
			
			cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(3, 0);
			metaffi::utils::scope_guard sg([&] { xllr_free_cdts_buffer(cdts_param_ret); });
			cdts& cdt_params = cdts_param_ret[0];
			cdt_params[0].set_handle(hthis);
			cdt_params[1].set_string((metaffi_string8)key.c_str(), true);
			cdt_metaffi_handle* mhandle = new cdt_metaffi_handle{val, (uint64_t)runtime_id, nullptr};
			cdt_params[2].set_handle(mhandle);

			char* err = nullptr;
			(*p_testmap_set)((cdts*)cdts_param_ret, &err);
			if(err)
			{
				FAIL(std::string(err));
			}
		};

		std::vector<int8_t> obj_to_insert1 = {1, 2, 3};
		std::vector<int8_t> obj_to_insert2 = {11, 12, 13};
		insert(p_testmap_set, testmap_instance, std::u8string(u8"key1"), &obj_to_insert1, 733);
		insert(p_testmap_set, testmap_instance, std::u8string(u8"key2"), &obj_to_insert2, 733);

		// contains
		entity_path = "callable=testmap.contains,instance_required";
		std::vector<metaffi_type_info> params_contains_types = {metaffi_type_info(metaffi_handle_type),
		                                                        metaffi_type_info(metaffi_string8_type)};
		std::vector<metaffi_type_info> retvals_contains_types = {metaffi_type_info(metaffi_bool_type)};

		xcall* p_testmap_contains = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_contains_types, retvals_contains_types);
		metaffi::utils::scope_guard sg7([&] {
			free_xcall(p_testmap_contains, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});
		
		cdts* cdts_param_ret2 = (cdts*)xllr_alloc_cdts_buffer(2, 1);
		metaffi::utils::scope_guard sg2([&] { xllr_free_cdts_buffer(cdts_param_ret2); });
		cdts& params = cdts_param_ret2[0];
		params[0].set_handle(testmap_instance);
		params[1].set_string(u8"key1", true);

		(*p_testmap_contains)((cdts*)cdts_param_ret2);
		if(err)
		{
			FAIL(std::string(err));
		}
		
		cdts& wrapper_contains_ret = cdts_param_ret2[1];
		REQUIRE((wrapper_contains_ret[0].type == metaffi_bool_type));
		REQUIRE((wrapper_contains_ret[0].cdt_val.bool_val != 0));

		// get
		entity_path = "callable=testmap.get,instance_required";
		std::vector<metaffi_type_info> params_get_types = {metaffi_type_info(metaffi_handle_type),
		                                                   metaffi_type_info(metaffi_string8_type)};
		std::vector<metaffi_type_info> retvals_get_types = {metaffi_type_info(metaffi_any_type)};

		xcall* p_testmap_get = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_get_types, retvals_get_types);
		metaffi::utils::scope_guard sg8([&] {
			free_xcall(p_testmap_get, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		auto get = [&p_testmap_get](const cdt_metaffi_handle* hthis, const std::u8string& key) -> std::vector<int8_t> {
			cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(2, 1);
			metaffi::utils::scope_guard sg([&] { xllr_free_cdts_buffer(cdts_param_ret); });
			cdts& wrapper_get_params = cdts_param_ret[0];
			wrapper_get_params[0].set_handle(hthis);
			wrapper_get_params[1].set_string((metaffi_string8)key.c_str(), true);

			char* err = nullptr;
			(*p_testmap_get)((cdts*)cdts_param_ret, &err);
			if(err)
			{
				FAIL(std::string(err));
			}

			cdts& wrapper_get_ret = cdts_param_ret[1];
			REQUIRE((wrapper_get_ret[0].type == metaffi_handle_type));
			REQUIRE((wrapper_get_ret[0].cdt_val.handle_val->runtime_id == 733));
			std::vector<int8_t>* object_pulled = (std::vector<int8_t>*)wrapper_get_ret[0].cdt_val.handle_val->handle;
			return *object_pulled;
		};

		std::vector<int8_t> inner_obj1 = get(testmap_instance, std::u8string(u8"key1"));
		REQUIRE((inner_obj1 == std::vector<int8_t>{1, 2, 3}));

		std::vector<int8_t> inner_obj2 = get(testmap_instance, std::u8string(u8"key2"));
		REQUIRE((inner_obj2 == std::vector<int8_t>{11, 12, 13}));
	}

	TEST_CASE("runtime_test_target.testmap.get_set_name")
	{
		// Load constructor
		std::string entity_path = "callable=testmap";
		std::vector<metaffi_type_info> retvals_types = {metaffi_type_info(metaffi_handle_type)};

		xcall* pnew_testmap = cpp_load_entity(setup.runtime_test_target_path, entity_path, {}, retvals_types);
		metaffi::utils::scope_guard sg5([&] {
			free_xcall(pnew_testmap, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		// Load getter
		entity_path = "attribute=name,instance_required,getter";
		std::vector<metaffi_type_info> params_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}};

		xcall* pget_name = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_types, retvals_types);
		metaffi::utils::scope_guard sg6([&] {
			free_xcall(pget_name, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		// Load setter
		entity_path = "attribute=name,instance_required,setter";
		params_types = {metaffi_type_info{metaffi_handle_type}, metaffi_type_info{metaffi_string8_type}};

		xcall* pset_name = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_types, {});
		metaffi::utils::scope_guard sg7([&] {
			free_xcall(pset_name, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		// Create new testmap
		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg([&] { xllr_free_cdts_buffer(cdts_param_ret); });
		(*pnew_testmap)((cdts*)cdts_param_ret, &err);

		// Check for errors
		if(err)
		{
			FAIL(std::string(err));
		}

		REQUIRE((cdts_param_ret[1].arr[0].type == metaffi_handle_type));
		REQUIRE((cdts_param_ret[1].arr[0].cdt_val.handle_val->handle != nullptr));

		cdt_metaffi_handle* testmap_instance = cdts_param_ret[1].arr[0].cdt_val.handle_val;

		// Get name
		cdts* cdts_param_ret2 = (cdts*)xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg2([&cdts_param_ret2]() { xllr_free_cdts_buffer(cdts_param_ret2); });
		cdts_param_ret2[0].arr[0].set_handle(testmap_instance);

		(*pget_name)((cdts*)cdts_param_ret2, &err);
		// Check for errors
		if(err)
		{
			FAIL(std::string(err));
		}

		REQUIRE((cdts_param_ret2[1].arr[0].type == metaffi_string8_type));
		REQUIRE((std::u8string(cdts_param_ret2[1].arr[0].cdt_val.string8_val) == u8"name1"));

		// Set name to "name is my name"
		cdts* cdts_param_ret3 = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		metaffi::utils::scope_guard sg3([&cdts_param_ret3]() { xllr_free_cdts_buffer(cdts_param_ret3); });
		cdts_param_ret3[0].arr[0].set_handle(testmap_instance);
		cdts_param_ret3[0].arr[1].set_string(u8"name is my name", true);

		(*pset_name)((cdts*)cdts_param_ret3, &err);
		// Check for errors
		if(err)
		{
			FAIL(std::string(err));
		}

		// Get name again and make sure it is "name is my name"
		cdts* last_get_params = (cdts*)xllr_alloc_cdts_buffer(1, 1);
		metaffi::utils::scope_guard sg4([&last_get_params]() { xllr_free_cdts_buffer(last_get_params); });
		last_get_params[0].arr[0].set_handle(testmap_instance);

		(*pget_name)((cdts*)last_get_params, &err);
		// Check for errors
		if(err)
		{
			FAIL(std::string(err));
		}
		
		REQUIRE((last_get_params[1].arr[0].type == metaffi_string8_type));
		REQUIRE((std::u8string(last_get_params[1].arr[0].cdt_val.string8_val) == u8"name is my name"));
	}

	TEST_CASE("runtime_test_target.SomeClass")
	{
		std::string entity_path = "callable=get_some_classes";
		std::vector<metaffi_type_info> retvals_getSomeClasses_types = {metaffi_type_info{metaffi_handle_array_type}};

		xcall* pgetSomeClasses = cpp_load_entity(setup.runtime_test_target_path, entity_path, {}, retvals_getSomeClasses_types);
		metaffi::utils::scope_guard sg4([&] {
			free_xcall(pgetSomeClasses, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		entity_path = "callable=expect_three_some_classes";
		std::vector<metaffi_type_info> params_expectThreeSomeClasses_types = {metaffi_type_info{metaffi_handle_array_type}};

		xcall* pexpectThreeSomeClasses = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_expectThreeSomeClasses_types, {});
		metaffi::utils::scope_guard sg5([&] {
			free_xcall(pexpectThreeSomeClasses, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		entity_path = "callable=SomeClass.print";
		std::vector<metaffi_type_info> params_SomeClassPrint_types = {metaffi_type_info{metaffi_handle_type}};

		xcall* pSomeClassPrint = cpp_load_entity(setup.runtime_test_target_path, entity_path, params_SomeClassPrint_types, {});
		metaffi::utils::scope_guard sg6([&] {
			free_xcall(pSomeClassPrint, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg([&] { xllr_free_cdts_buffer(cdts_param_ret); });
		cdts& res = cdts_param_ret[1];

		(*pgetSomeClasses)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		REQUIRE(((res[0].type == metaffi_handle_array_type)));
		REQUIRE(((res[0].cdt_val.array_val->length == 3)));
		REQUIRE((res[0].cdt_val.array_val->arr[0].cdt_val.handle_val->handle != nullptr));
		REQUIRE((res[0].cdt_val.array_val->arr[1].cdt_val.handle_val->handle != nullptr));
		REQUIRE((res[0].cdt_val.array_val->arr[2].cdt_val.handle_val->handle != nullptr));

		std::vector<cdt_metaffi_handle*> some_classes = {res[0].cdt_val.array_val->arr[0].cdt_val.handle_val,
		                                                res[0].cdt_val.array_val->arr[1].cdt_val.handle_val,
		                                                res[0].cdt_val.array_val->arr[2].cdt_val.handle_val};

		// call print on each SomeClass
		cdts* cdts_param_ret3 = (cdts*)xllr_alloc_cdts_buffer(1, 0);
		metaffi::utils::scope_guard sg2([&cdts_param_ret3]() { xllr_free_cdts_buffer(cdts_param_ret3); });
		cdts& print_params = cdts_param_ret3[0];
		print_params[0].set_handle(some_classes[0]);

		(*pSomeClassPrint)((cdts*)cdts_param_ret3, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		print_params[0].set_handle(some_classes[1]);
		(*pSomeClassPrint)((cdts*)cdts_param_ret3, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		print_params[0].set_handle(some_classes[2]);
		(*pSomeClassPrint)((cdts*)cdts_param_ret3, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		cdts* cdts_param_ret2 = (cdts*)xllr_alloc_cdts_buffer(1, 0);
		metaffi::utils::scope_guard sg3([&cdts_param_ret2]() { xllr_free_cdts_buffer(cdts_param_ret2); });
		cdts& params = cdts_param_ret2[0];
		params[0].set_new_array(3, 1, metaffi_handle_type);
		params[0].cdt_val.array_val->arr[0].set_handle(some_classes[0]);
		params[0].cdt_val.array_val->arr[1].set_handle(some_classes[1]);
		params[0].cdt_val.array_val->arr[2].set_handle(some_classes[2]);

		(*pexpectThreeSomeClasses)((cdts*)cdts_param_ret2, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
	}


	TEST_CASE("runtime_test_target.ThreeBuffers")
	{
		std::string entity_path = "callable=expect_three_buffers";
		xcall* pexpectThreeBuffers = cpp_load_entity(setup.runtime_test_target_path, entity_path, {metaffi_type_info{metaffi_uint8_array_type, nullptr, false, 2}}, {});
		metaffi::utils::scope_guard sg10([&] {
			free_xcall(pexpectThreeBuffers, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});

		entity_path = "callable=get_three_buffers";
		xcall* pgetThreeBuffers = cpp_load_entity(setup.runtime_test_target_path, entity_path, {}, {{metaffi_uint8_array_type, nullptr, false, 2}});
		metaffi::utils::scope_guard sg11([&] {
			free_xcall(pgetThreeBuffers, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});
		
		// pass 3 buffers
		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(1, 0);
		metaffi::utils::scope_guard sg([&cdts_param_ret]() { xllr_free_cdts_buffer(cdts_param_ret); });
		cdts& params = cdts_param_ret[0];
		cdts& ret = cdts_param_ret[1];
		params[0].set_new_array(3, 2, metaffi_uint8_array_type);
		params[0].cdt_val.array_val->arr[0].set_new_array(2, 1, metaffi_uint8_array_type);
		params[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0] = ((metaffi_uint8)1);
		params[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1] = ((metaffi_uint8)2);

		params[0].cdt_val.array_val->arr[1].set_new_array(3, 1, metaffi_uint8_array_type);
		params[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0] = ((metaffi_uint8)3);
		params[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1] = ((metaffi_uint8)4);
		params[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2] = ((metaffi_uint8)5);

		params[0].cdt_val.array_val->arr[2].set_new_array(4, 1, metaffi_uint8_array_type);
		params[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[0] = ((metaffi_uint8)6);
		params[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[1] = ((metaffi_uint8)7);
		params[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[2] = ((metaffi_uint8)8);
		params[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[3] = ((metaffi_uint8)9);

		(*pexpectThreeBuffers)((cdts*)cdts_param_ret, &err);

		if(err)
		{
			FAIL(std::string(err));
		}

		// get 3 buffers
		cdts* get_three_buffers_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg2([&get_three_buffers_ret]() { xllr_free_cdts_buffer(get_three_buffers_ret); });
		(*pgetThreeBuffers)(get_three_buffers_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
		cdts& ret2 = get_three_buffers_ret[1];
		REQUIRE((ret2[0].type == metaffi_uint8_array_type));
		REQUIRE((ret2[0].cdt_val.array_val->fixed_dimensions == 2));
		REQUIRE((ret2[0].cdt_val.array_val->length == 3));

		REQUIRE((ret2[0].cdt_val.array_val->arr[0].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((ret2[0].cdt_val.array_val->arr[0].cdt_val.array_val->length == 4));
		REQUIRE((ret2[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[0].cdt_val.uint8_val == 1));
		REQUIRE((ret2[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[1].cdt_val.uint8_val == 2));
		REQUIRE((ret2[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[2].cdt_val.uint8_val == 3));
		REQUIRE((ret2[0].cdt_val.array_val->arr[0].cdt_val.array_val->arr[3].cdt_val.uint8_val == 4));

		REQUIRE((ret2[0].cdt_val.array_val->arr[1].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((ret2[0].cdt_val.array_val->arr[1].cdt_val.array_val->length == 3));
		REQUIRE((ret2[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[0].cdt_val.uint8_val == 5));
		REQUIRE((ret2[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[1].cdt_val.uint8_val == 6));
		REQUIRE((ret2[0].cdt_val.array_val->arr[1].cdt_val.array_val->arr[2].cdt_val.uint8_val == 7));

		REQUIRE((ret2[0].cdt_val.array_val->arr[2].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((ret2[0].cdt_val.array_val->arr[2].cdt_val.array_val->length == 2));
		REQUIRE((ret2[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[0].cdt_val.uint8_val == 8));
		REQUIRE((ret2[0].cdt_val.array_val->arr[2].cdt_val.array_val->arr[1].cdt_val.uint8_val == 9));
	}


	TEST_CASE("runtime_test_target.set_five_seconds")
	{
		// set five_seconds global;
		std::string variable_path = "attribute=five_seconds,setter";
		xcall* pfive_seconds_setter = cpp_load_entity(setup.runtime_test_target_path, variable_path, {metaffi_type_info(metaffi_int64_type)}, {});
		metaffi::utils::scope_guard sg5([&] {
			free_xcall(pfive_seconds_setter, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});
		
		cdts* setter_ret = (cdts*)xllr_alloc_cdts_buffer(1, 0);
		metaffi::utils::scope_guard sg([&setter_ret]() { xllr_free_cdts_buffer(setter_ret); });
		cdts& params = setter_ret[0];
		params[0] = ((metaffi_int64)10);

		(*pfive_seconds_setter)((cdts*)setter_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		// get five_seconds global
		std::string getter_variable_path = "attribute=five_seconds,getter";
		xcall* pfive_seconds_getter = cpp_load_entity(setup.runtime_test_target_path, getter_variable_path, {}, {metaffi_type_info(metaffi_int64_type)});
		metaffi::utils::scope_guard sg6([&] {
			free_xcall(pfive_seconds_getter, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});
		
		cdts* getter_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg2([&getter_ret]() { xllr_free_cdts_buffer(getter_ret); });
		cdts& ret = getter_ret[1];

		(*pfive_seconds_getter)((cdts*)getter_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		REQUIRE((ret[0].type == metaffi_int64_type));
		REQUIRE((ret[0].cdt_val.int64_val == 10));
	}

	TEST_CASE("return_null")
	{
		xcall* preturn_null = cpp_load_entity(setup.runtime_test_target_path, "callable=return_null", {}, {metaffi_type_info{metaffi_handle_type}});
		metaffi::utils::scope_guard sg5([&] {
			free_xcall(preturn_null, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});
		
		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg([&cdts_param_ret]() { xllr_free_cdts_buffer(cdts_param_ret); });
		cdts& ret = cdts_param_ret[1];

		(*preturn_null)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
		REQUIRE((ret[0].type == metaffi_null_type));
	}

	TEST_CASE("returns_array_of_different_objects")
	{
		// returns [1, 'string', 3.0, None, bytes([1, 2, 3]), SomeClass()]
		xcall* preturns_array_of_different_objects = cpp_load_entity(setup.runtime_test_target_path,
		                                                             "callable=returns_array_of_different_objects",
		                                                             {}, {metaffi_type_info{metaffi_any_type}});
		metaffi::utils::scope_guard sg10([&] {
			free_xcall(preturns_array_of_different_objects, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});
		
		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg([&cdts_param_ret]() { xllr_free_cdts_buffer(cdts_param_ret); });
		cdts& ret = cdts_param_ret[1];

		(*preturns_array_of_different_objects)((cdts*)cdts_param_ret, &err);

		REQUIRE(((ret[0].type & metaffi_array_type) != 0));
		REQUIRE((ret[0].cdt_val.array_val->fixed_dimensions == MIXED_OR_UNKNOWN_DIMENSIONS));
		REQUIRE((ret[0].cdt_val.array_val->length == 6));

		REQUIRE((ret[0].cdt_val.array_val->arr[0].type == metaffi_int64_type));
		REQUIRE((ret[0].cdt_val.array_val->arr[1].type == metaffi_string8_type));
		REQUIRE((ret[0].cdt_val.array_val->arr[2].type == metaffi_float64_type));
		REQUIRE((ret[0].cdt_val.array_val->arr[3].type == metaffi_null_type));
		REQUIRE((ret[0].cdt_val.array_val->arr[4].type == metaffi_uint8_array_type));
		REQUIRE((ret[0].cdt_val.array_val->arr[5].type == metaffi_handle_type));

		REQUIRE((ret[0].cdt_val.array_val->arr[0].cdt_val.int64_val == 1));
		REQUIRE((std::u8string(ret[0].cdt_val.array_val->arr[1].cdt_val.string8_val) == u8"string"));
		REQUIRE((ret[0].cdt_val.array_val->arr[2].cdt_val.float64_val == 3.0));
		REQUIRE((ret[0].cdt_val.array_val->arr[4].cdt_val.array_val->length == 3));
		REQUIRE((ret[0].cdt_val.array_val->arr[4].cdt_val.array_val->fixed_dimensions == 1));
		REQUIRE((ret[0].cdt_val.array_val->arr[4].cdt_val.array_val->arr[0].cdt_val.uint8_val == 1));
		REQUIRE((ret[0].cdt_val.array_val->arr[4].cdt_val.array_val->arr[1].cdt_val.uint8_val == 2));
		REQUIRE((ret[0].cdt_val.array_val->arr[4].cdt_val.array_val->arr[2].cdt_val.uint8_val == 3));
		REQUIRE((ret[0].cdt_val.array_val->arr[5].cdt_val.handle_val->handle != nullptr));
		REQUIRE((((PyObject*)(ret[0].cdt_val.array_val->arr[5].cdt_val.handle_val->handle))->ob_refcnt > 0));
	}

	TEST_CASE("call_any")
	{
		// call with 1, 'string', 3.0, null, bytes([1, 2, 3]), SomeClass()
		xcall* pcall_any = cpp_load_entity(setup.runtime_test_target_path, "callable=accepts_any",
		                                   {metaffi_type_info{metaffi_int64_type}, metaffi_type_info{metaffi_any_type}}, {});
		metaffi::utils::scope_guard sg10([&] {
			free_xcall(pcall_any, &err);

			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});
		
		xcall* pnew_someclass = cpp_load_entity(setup.runtime_test_target_path, "callable=SomeClass",
		                                        {}, {metaffi_type_info(metaffi_handle_type)});
		metaffi::utils::scope_guard sg11([&] {
			free_xcall(pnew_someclass, &err);
			
			if(err)
			{
				std::cerr << err << std::endl;
				FAIL(std::string(err));
			}
		});


		cdts* cdts_param_ret = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		metaffi::utils::scope_guard sg([&cdts_param_ret]() { xllr_free_cdts_buffer(cdts_param_ret); });
		cdts& params = cdts_param_ret[0];
		params[0] = (metaffi_int64)0;
		params[1] = (metaffi_int64)1;
		
		(*pcall_any)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		cdts* cdts_param_ret2 = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		metaffi::utils::scope_guard sg2([&cdts_param_ret2]() { xllr_free_cdts_buffer(cdts_param_ret2); });
		cdts& params2 = cdts_param_ret2[0];
		params2[0] = (metaffi_int64)1;
		params2[1].set_string((metaffi_string8)u8"string", true);
		(*pcall_any)((cdts*)cdts_param_ret2, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		cdts* cdts_param_ret3 = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		metaffi::utils::scope_guard sg3([&cdts_param_ret3]() { xllr_free_cdts_buffer(cdts_param_ret3); });
		cdts& params3 = cdts_param_ret3[0];
		params3[0] = (metaffi_int64)2;
		params3[1] = (metaffi_float64)3.0;
		(*pcall_any)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		cdts* cdts_param_ret4 = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		metaffi::utils::scope_guard sg4([&cdts_param_ret4]() { xllr_free_cdts_buffer(cdts_param_ret4); });
		cdts& params4 = cdts_param_ret4[0];
		params4[0] = (metaffi_int64)3;
		params4[1].cdt_val.handle_val = nullptr;
		(*pcall_any)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}

		cdts* cdts_param_ret5 = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		metaffi::utils::scope_guard sg5([&cdts_param_ret5]() { xllr_free_cdts_buffer(cdts_param_ret5); });
		cdts& params5 = cdts_param_ret5[0];
		params5[0] = ((metaffi_int64)4);
		params5[1].set_new_array(3, 1, metaffi_uint8_array_type);
		params5[1].cdt_val.array_val->arr[0] = ((metaffi_uint8)1);
		params5[1].cdt_val.array_val->arr[1] = ((metaffi_uint8)2);
		params5[1].cdt_val.array_val->arr[2] = ((metaffi_uint8)3);
		(*pcall_any)((cdts*)cdts_param_ret, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
		
		// new some class
		cdts* cdts_param_ret7 = (cdts*)xllr_alloc_cdts_buffer(0, 1);
		metaffi::utils::scope_guard sg7([&cdts_param_ret7]() { xllr_free_cdts_buffer(cdts_param_ret7); });
		cdts& someclass_ret = cdts_param_ret7[1];
		(*pnew_someclass)((cdts*)cdts_param_ret7, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
		REQUIRE((someclass_ret[0].type == metaffi_handle_type));

		auto someclass_instance = someclass_ret[0].cdt_val.handle_val;

		cdts* cdts_param_ret6 = (cdts*)xllr_alloc_cdts_buffer(2, 0);
		metaffi::utils::scope_guard sg6([&cdts_param_ret6]() { xllr_free_cdts_buffer(cdts_param_ret6); });
		cdts& params6 = cdts_param_ret6[0];
		params6[0] = ((metaffi_int64)5);
		params6[1].set_handle(someclass_instance);
		(*pcall_any)((cdts*)cdts_param_ret3, &err);
		if(err)
		{
			FAIL(std::string(err));
		}
	}

	TEST_CASE("!call_callback_add")
	{
	}

	TEST_CASE("!return_multiple_return_values")
	{
	}

	TEST_CASE("!extended_test - test different python calling methods + properties")
	{
	}
}


TEST_SUITE("libraries")
{

	TEST_CASE("!collections.deque")
	{
	}

	TEST_CASE("!3rd party - beautifulsoup4")
	{
	}

	TEST_CASE("!3rd party - numpy")
	{
	}

	TEST_CASE("!3rd party - pandas")
	{
	}
}