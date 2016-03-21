% include header

<div class="container-fluid">

  <h1 class="text-center"> Create structure </h1>

  <form action="/struct_do_create?" method="post" class="form-inline">
    <table class="table table-bordered">
      <tr>
	<td> name </td>
	<td> size </td>
	<td> </td>
      </tr>
      <tr>
	<td> <input type="text" name="name" value="name" class="form-control" /> </td>
	<td> <input type="number" name="size" value="1" class="form-control" /> </td>
	<td> <input type="submit" value="Create" class="btn btn-primary" /> </td>
      </tr>
    </table>
  </form>
  
</div>
% include footer 
