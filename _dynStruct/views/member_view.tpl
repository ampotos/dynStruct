% include header
<div class="container-fluid">
  <h1 class="text-center"> {{! name_member}} informations</h1>
% if not edit:
  <h3 class="text-center"> <a href="/member_edit?id_struct={{id_member[:id_member.rfind('.')]}}&id_member={{member.offset}}"> Edit member </a> </h3>
% end
  
% if member.is_array:

% if not edit:
  <table class="table table-bordered">
    <tr>
      <td> number of units </td>
      <td> size of unit (in bytes)</td>
      <td> unit type </td>
      <td> total size (in bytes) </td>
    </tr>
    <tr>
      <td> {{member.number_unit}} </td>
      <td> {{member.size_unit}} </td>
      <td> <span class="text-warning">{{member.t}}</span></td>
      <td> {{member.size}} </td>
    </tr>
  </table>
% else:
  <form action="/member_do_edit?id_struct={{id_member[:id_member.rfind('.')]}}&id_member={{member.offset}}" method="post" class="form-inline">
    <table class="table table-bordered">
      <tr>
	<td> name </td>
	<td> number of units </td>
	<td> size of unit (in bytes)</td>
	<td> unit type </td>
	<td> </td>
      </tr>
      <tr>
	<td> <input type="text" name="name" value="{{member.name}}" class="form-control"> </td>
	<td> <input type="number" name="nb_unit" value="{{member.number_unit}}" class="form-control"> </td>
	<td> <input type="number" name="size_unit" value="{{member.size_unit}}" class="form-control"> </td>
	<td> <input type="text" name="type" value="{{member.t}}" class="form-control"> </td>
	<td> <input type="submit" value="Edit" class="btn btn-primary"> </td>
      </tr>
    </table>
  </form>
% end
  
% elif member.is_struct:
% if edit:
  <form action="/member_do_edit?id_struct={{id_member[:id_member.rfind('.')]}}&id_member={{member.offset}}" method="post" class="form-inline">
    <table class="table table-bordered">
      <tr>
	<td> name </td>
	<td> size </td>
	<td> </td>
      </tr>
      <tr>
	<td> <input type="text" name="name" value="{{member.name}}" class="form-control"> </td>
	<td> <input type="number" name="nb_unit" value="{{member.number_unit}}" class="form-control"> </td>
	<td> <input type="number" name="size_unit" value="{{member.size_unit}}" class="form-control"> </td>
	<td> <input type="text" name="type" value="{{member.sub_struct.name}}" class="form-control"> </td>
	<td> <input type="submit" value="Edit" class="btn btn-primary"> </td>
      </tr>
    </table>
    </form>
% end
  <h1 class="text-center"> Structure members</h1>
% include('member_search.tpl', in_page=True, id_struct=id_member)

% elif member.is_array_struct:

% if not edit:
  <table class="table table-bordered">
    <tr>
      <td> number of units </td>
      <td> size of unit (in bytes)</td>
      <td> unit type </td>
      <td> total size (in bytes) </td>
    </tr>
    <tr>
      <td> {{member.number_unit}} </td>
      <td> {{member.size_unit}} </td>
      <td> <span class="text-warning">{{member.sub_struct.name}}</span></td>
      <td> {{member.size}} </td>
    </tr>
  </table>
% else:
  <form action="/member_do_edit?id_struct={{id_member[:id_member.rfind('.')]}}&id_member={{member.offset}}" method="post" class="form-inline">
    <table class="table table-bordered">
      <tr>
	<td> name </td>
	<td> number of units </td>
	<td> size of unit (or size of struct)</td>
	<td> name of struct </td>
	<td> </td>
      </tr>
      <tr>
	<td> <input type="text" name="name" value="{{member.name}}" class="form-control"> </td>
	<td> <input type="number" name="nb_unit" value="{{member.number_unit}}" class="form-control"> </td>
	<td> <input type="number" name="size_unit" value="{{member.size_unit}}" class="form-control"> </td>
	<td> <input type="text" name="type" value="{{member.sub_struct.name}}" class="form-control"> </td>
	<td> <input type="submit" value="Edit" class="btn btn-primary"> </td>
      </tr>
    </table>
    </form>
% end
  <h1 class="text-center"> {{member.sub_struct.name}} members</h1>
% include('member_search.tpl', in_page=True, id_struct=id_member)
  
% else:

% if not edit:
  <table class="table table-bordered">
    <tr>
      <td> size (in bytes)</td>
      <td> type </td>
    </tr>
    <tr>
      <td> {{member.size}} </td>
      <td> <span class="text-warning">{{member.t}}</span></td>
    </tr>
  </table>
% else:
  <form action="/member_do_edit?id_struct={{id_member[:id_member.rfind('.')]}}&id_member={{member.offset}}" method="post" class="form-inline">
    <table class="table table-bordered">
      <tr>
	<td> name </td>
	<td> size (in bytes)</td>
	<td> type </td>
	<td> </td>
      </tr>
      <tr>
	<td> <input type="text" name="name" value="{{member.name}}" class="form-control"> </td>
	<td> <input type="number" name="size" value="{{member.size}}" class="form-control"> </td>
	<td> <input type="test" name="type" value="{{member.t}}" class="form-control"> </td>
	<td> <input type="submit" value="Edit" class="btn btn-primary"> </td>
      </tr>
    </table>
  </form>
% end
% end

  <h1 class="text-center"> Member access</h1>
% include('access_search.tpl', in_page=True, id_block=None, id_member=id_member)

</div>

% include footer
