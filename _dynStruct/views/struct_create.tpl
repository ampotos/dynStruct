% include header

<div class="container-fluid">

  <h1 class="text-center"> Create structure </h1>

  <form action="/struct_do_create?" method="post" class="form-inline text-center">
    <input type="text" name="name" value="name" class="form-control" />
    <input type="number" name="size" value="1" class="form-control" />
    <input type="submit" value="Create" class="btn btn-primary" />
  </form>
  
</div>
% include footer 
