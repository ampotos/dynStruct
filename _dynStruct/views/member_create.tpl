% include header
<script type="text/javascript">
  $(document).ready(function(){
      $("#select-tab a").click(function(e){
          e.preventDefault();
          $(this).tab('show');
      });
  })
</script>

<div class="container-fluid">

  <h1 class="text-center"> Member type </h1>

  <h3 class="text-center"> Padding offset : {{member.offset}} </h3>
  <h3 class="text-center"> Padding size : {{member.size}} </h3>
  
  <div class="text-center" id="select-tab">
    <ul class="nav nav-pills centered-pills">
      <li class="active"><a href="#simple">Simple member</a></li>
      <li><a href="#array">Array member</a></li>
      <li><a href="#struct">Struct member</a></li>
      <li><a href="#array_struct">Array of struct member</a></li>
    </ul>
  </div>

  <div class="tab-content">
    <div class="tab-pane active" id="simple">

      <form action="/member_do_create?id_struct={{id_struct}}&id_member={{member.offset}}" method="post" class="form-inline">
	<table class="table table-bordered">
	  <tr>
	    <td> name </td>
	    <td> offset </td>
	    <td> size (in bytes)</td>
	    <td> type </td>
	    <td> </td>
	  </tr>
	  <tr>
	    <td> <input type="text" name="name" value="name" class="form-control" /> </td>
	    <td> <input type="number" name="offset" value="{{member.offset}}" class="form-control" /> </td>
	    <td> <input type="number" name="size" value="1" class="form-control" /> </td>
	    <td> <input type="text" name="type" value="type" class="form-control" /> </td>
	    <input type="hidden" name="member_type" value="simple" />
	    <td> <input type="submit" value="Create" class="btn btn-primary" /> </td>
	  </tr>
	</table>
      </form>

    </div>
    
    <div class="tab-pane" id="array">

      <form action="/member_do_create?id_struct={{id_struct}}&id_member={{member.offset}}" method="post" class="form-inline">
	<table class="table table-bordered">
	  <tr>
	    <td> name </td>
	    <td> offset </td>
	    <td> number of units </td>
	    <td> size of unit (in bytes)</td>
	    <td> unit type </td>
	    <td> </td>
	  </tr>
	  <tr>
	    <td> <input type="text" name="name" value="name" class="form-control" /> </td>
	    <td> <input type="number" name="offset" value="{{member.offset}}" class="form-control" /> </td>
	    <td> <input type="number" name="nb_unit" value="1" class="form-control" /> </td>
	    <td> <input type="number" name="size_unit" value="1" class="form-control" /> </td>
	    <td> <input type="text" name="type" value="unit_type" class="form-control" /> </td>
	    <input type="hidden" name="member_type" value="array" />
	    <td> <input type="submit" value="Create" class="btn btn-primary" /> </td>
	  </tr>
	</table>
      </form>

    </div>

    <div class="tab-pane" id="struct">

      <form action="/member_do_create?id_struct={{id_struct}}&id_member={{member.offset}}" method="post" class="form-inline">
	<table class="table table-bordered">
	  <tr>
	    <td> name </td>
	    <td> offset </td>
	    <td> size (in bytes)</td>
	    <td> </td>
	  </tr>
	  <tr>
	    <td> <input type="text" name="name" value="name" class="form-control" /> </td>
	    <td> <input type="number" name="offset" value="{{member.offset}}" class="form-control" /> </td>
	    <td> <input type="number" name="size" value="1" class="form-control" /> </td>
	    <input type="hidden" name="member_type" value="struct" />
	    <td> <input type="submit" value="Create" class="btn btn-primary" /> </td>
	  </tr>
	</table>
      </form>

    </div>
    
    <div class="tab-pane" id="array_struct">

      <form action="/member_do_create?id_struct={{id_struct}}&id_member={{member.offset}}" method="post" class="form-inline">
	<table class="table table-bordered">
	  <tr>
	    <td> name </td>
	    <td> offset </td>
	    <td> number of units </td>
	    <td> size of unit (in bytes)</td>
	    <td> unit type </td>
	    <td> </td>
	  </tr>
	  <tr>
	    <td> <input type="text" name="name" value="name" class="form-control" /> </td>
	    <td> <input type="number" name="offset" value="{{member.offset}}" class="form-control" /> </td>
	    <td> <input type="number" name="nb_unit" value="1" class="form-control" /> </td>
	    <td> <input type="number" name="size_unit" value="1" class="form-control" /> </td>
	    <td> <input type="text" name="type" value="unit_type" class="form-control" /> </td>
	    <input type="hidden" name="member_type" value="array_struct" />
	    <td> <input type="submit" value="Create" class="btn btn-primary" /> </td>
	  </tr>
	</table>
      </form>

    </div>
	
  </div>

</div>

% include footer
